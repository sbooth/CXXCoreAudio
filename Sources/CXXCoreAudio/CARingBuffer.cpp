//
// Copyright © 2013-2025 Stephen F. Booth
// Part of https://github.com/sbooth/CXXCoreAudio
// MIT license
//

#import <algorithm>
#import <cassert>
#import <cstdlib>
#import <cstring>
#import <limits>
#import <new>
#import <stdexcept>
#import <utility>

#import "CARingBuffer.hpp"

namespace {

/// Calculates and returns the smallest integral power of two not less than x.
/// @param x A value on the closed interval [0, 2147483648].
/// @return The smallest integral power of two not less than x.
constexpr uint32_t bit_ceil(uint32_t x) noexcept
{
	if(x < 2)
		return 1;
	const auto n = std::numeric_limits<uint32_t>::digits - __builtin_clz(x - 1);
	assert(n != std::numeric_limits<uint32_t>::digits);
	return uint32_t{1} << n;
}

} /* namespace */

// MARK: Creation and Destruction

CXXCoreAudio::CARingBuffer::CARingBuffer(const AudioStreamBasicDescription& format, uint32_t size)
{
	if((format.mFormatFlags & kAudioFormatFlagIsNonInterleaved) == 0 || format.mBytesPerFrame == 0 || format.mChannelsPerFrame == 0) [[unlikely]]
		throw std::invalid_argument("unsupported audio format");
	if(size < 2 || size > 0x80000000) [[unlikely]]
		throw std::invalid_argument("capacity out of range");
	if(!Allocate(format, size)) [[unlikely]]
		throw std::bad_alloc();
}

CXXCoreAudio::CARingBuffer::CARingBuffer(CARingBuffer&& other) noexcept
: buffers_{std::exchange(other.buffers_, nullptr)}, capacity_{std::exchange(other.capacity_, 0)}, capacityMask_{std::exchange(other.capacityMask_, 0)}, timeBoundsQueueCounter_{other.timeBoundsQueueCounter_.exchange(0, std::memory_order_relaxed)}, format_{std::exchange(other.format_, {})}
{
	for(uint32_t i = 0; i < sTimeBoundsQueueSize; ++i) {
		timeBoundsQueue_[i].startTime_ = std::exchange(other.timeBoundsQueue_[i].startTime_, 0);
		timeBoundsQueue_[i].endTime_ = std::exchange(other.timeBoundsQueue_[i].endTime_, 0);
		timeBoundsQueue_[i].updateCounter_.store(other.timeBoundsQueue_[i].updateCounter_.exchange(0, std::memory_order_relaxed), std::memory_order_relaxed);
	}
}

CXXCoreAudio::CARingBuffer& CXXCoreAudio::CARingBuffer::operator=(CARingBuffer&& other) noexcept
{
	if(this != &other) [[unlikely]] {
		std::free(buffers_);
		buffers_ = std::exchange(other.buffers_, nullptr);
		capacity_ = std::exchange(other.capacity_, 0);
		capacityMask_ = std::exchange(other.capacityMask_, 0);
		for(uint32_t i = 0; i < sTimeBoundsQueueSize; ++i) {
			timeBoundsQueue_[i].startTime_ = std::exchange(other.timeBoundsQueue_[i].startTime_, 0);
			timeBoundsQueue_[i].endTime_ = std::exchange(other.timeBoundsQueue_[i].endTime_, 0);
			timeBoundsQueue_[i].updateCounter_.store(other.timeBoundsQueue_[i].updateCounter_.exchange(0, std::memory_order_relaxed), std::memory_order_relaxed);
		}
		timeBoundsQueueCounter_.store(other.timeBoundsQueueCounter_.exchange(0, std::memory_order_relaxed), std::memory_order_relaxed);
		format_ = std::exchange(other.format_, {});
	}
	return *this;
}

CXXCoreAudio::CARingBuffer::~CARingBuffer() noexcept
{
	std::free(buffers_);
}

// MARK: Buffer Management

bool CXXCoreAudio::CARingBuffer::Allocate(const AudioStreamBasicDescription& format, uint32_t size) noexcept
{
	if((format.mFormatFlags & kAudioFormatFlagIsNonInterleaved) == 0 || format.mBytesPerFrame == 0 || format.mChannelsPerFrame == 0) [[unlikely]]
		return false;
	if(size < 2 || size > 0x80000000) [[unlikely]]
		return false;

	/// Values larger than this will overflow AudioBuffer.mDataByteSize
	const auto maxAudioBufferFrameCount = std::numeric_limits<UInt32>::max() / format.mBytesPerFrame;
	/// Values larger than this will exceed the maximum allocation size
	const auto maxAllocationFrameCount = ((std::numeric_limits<size_t>::max() / format.mChannelsPerFrame) - sizeof(void *)) / format.mBytesPerFrame;

	/// The maximum size per channel buffer in audio frames
	const auto maxChannelBufferFrameSize = std::min(static_cast<size_t>(maxAudioBufferFrameCount), maxAllocationFrameCount);

	// Round to nearest power of two
	const auto channelBufferFrameSize = bit_ceil(size);
	if(channelBufferFrameSize > maxChannelBufferFrameSize)
		return false;

	Deallocate();

	const auto channelBufferByteSize = channelBufferFrameSize * format.mBytesPerFrame;
	const auto allocationSize = (channelBufferByteSize + sizeof(void *)) * format.mChannelsPerFrame;

	auto allocation = std::malloc(allocationSize);
	if(!allocation) [[unlikely]]
		return false;

	// Zero the entire allocation
	std::memset(allocation, 0, allocationSize);

	// Assign the channel buffers
	auto address = reinterpret_cast<uintptr_t>(allocation);

	buffers_ = reinterpret_cast<void **>(address);
	address += format.mChannelsPerFrame * sizeof(void *);
	for(UInt32 i = 0; i < format.mChannelsPerFrame; ++i) {
		buffers_[i] = reinterpret_cast<void *>(address);
		address += channelBufferByteSize;
	}

	capacity_ = channelBufferFrameSize;
	capacityMask_ = channelBufferFrameSize - 1;

	format_ = format;

	// Zero the time bounds queue
	for(uint32_t i = 0; i < sTimeBoundsQueueSize; ++i) {
		timeBoundsQueue_[i].startTime_ = 0;
		timeBoundsQueue_[i].endTime_ = 0;
		timeBoundsQueue_[i].updateCounter_.store(0, std::memory_order_relaxed);
	}
	timeBoundsQueueCounter_.store(0, std::memory_order_relaxed);

	return true;
}

void CXXCoreAudio::CARingBuffer::Deallocate() noexcept
{
	if(buffers_) [[likely]] {
		std::free(buffers_);
		buffers_ = nullptr;

		capacity_ = 0;
		capacityMask_ = 0;

		for(uint32_t i = 0; i < sTimeBoundsQueueSize; ++i) {
			timeBoundsQueue_[i].startTime_ = 0;
			timeBoundsQueue_[i].endTime_ = 0;
			timeBoundsQueue_[i].updateCounter_.store(0, std::memory_order_relaxed);
		}
		timeBoundsQueueCounter_.store(0, std::memory_order_relaxed);

		format_.Reset();
	}
}

void CXXCoreAudio::CARingBuffer::Reset() noexcept
{
	for(uint32_t i = 0; i < sTimeBoundsQueueSize; ++i) {
		timeBoundsQueue_[i].startTime_ = 0;
		timeBoundsQueue_[i].endTime_ = 0;
		timeBoundsQueue_[i].updateCounter_.store(0, std::memory_order_relaxed);
	}
	timeBoundsQueueCounter_.store(0, std::memory_order_relaxed);
}

uint32_t CXXCoreAudio::CARingBuffer::Capacity() const noexcept
{
	return capacityMask_;
}

bool CXXCoreAudio::CARingBuffer::GetTimeBounds(int64_t& startTime, int64_t& endTime) const noexcept
{
	for(auto i = 0; i < 8; ++i) {
		const auto currentCounter = timeBoundsQueueCounter_.load(std::memory_order_acquire);
		const auto currentIndex = currentCounter & sTimeBoundsQueueMask;

		const TimeBounds& bounds = timeBoundsQueue_[currentIndex];
		if(const auto counter = bounds.updateCounter_.load(std::memory_order_acquire); counter == currentCounter) {
			startTime = bounds.startTime_;
			endTime = bounds.endTime_;
			return true;
		}
	}

	return false;
}

uint32_t CXXCoreAudio::CARingBuffer::UnusedSpace() const noexcept
{
	int64_t start, end;
	if(capacity_ == 0 || !GetTimeBounds(start, end))
		return 0;
	return capacity_ - static_cast<uint32_t>(end - start) - 1;
}

// MARK: Writing and Reading Audio

bool CXXCoreAudio::CARingBuffer::Write(const AudioBufferList * const bufferList, uint32_t frameCount, int64_t sampleTime) noexcept
{
	if(frameCount == 0) [[unlikely]]
		return true;

	if(!bufferList || frameCount > capacity_ || sampleTime < 0) [[unlikely]]
		return false;

	/// Returns the ring buffer's starting sample time.
	const auto startTime = [&]() noexcept -> int64_t {
		return timeBoundsQueue_[timeBoundsQueueCounter_.load(std::memory_order_acquire) & sTimeBoundsQueueMask].startTime_;
	};

	/// Returns the buffer's ring ending sample time.
	const auto endTime = [&]() noexcept -> int64_t {
		return timeBoundsQueue_[timeBoundsQueueCounter_.load(std::memory_order_acquire) & sTimeBoundsQueueMask].endTime_;
	};

	/// Sets the ring buffer's start and end sample times.
	const auto setTimeBounds = [&](int64_t startTime, int64_t endTime) noexcept {
		const auto nextCounter = timeBoundsQueueCounter_.load(std::memory_order_acquire) + 1;
		const auto nextIndex = nextCounter & sTimeBoundsQueueMask;

		timeBoundsQueue_[nextIndex].startTime_ = startTime;
		timeBoundsQueue_[nextIndex].endTime_ = endTime;
		timeBoundsQueue_[nextIndex].updateCounter_.store(nextCounter, std::memory_order_release);

		timeBoundsQueueCounter_.fetch_add(1, std::memory_order_release);
	};

	const auto endWrite = sampleTime + static_cast<int64_t>(frameCount);

	// Going backwards, throw everything out
	if(sampleTime < endTime())
		setTimeBounds(sampleTime, sampleTime);
	// The buffer has not yet wrapped and will not need to
	else if(endWrite - startTime() <= static_cast<int64_t>(capacity_))
		;
	// Advance the start time past the region about to be overwritten
	else {
		const int64_t newStart = endWrite - static_cast<int64_t>(capacity_);	// one buffer of time behind the write position
		const int64_t newEnd = std::max(newStart, endTime());
		setTimeBounds(newStart, newEnd);
	}

	uint32_t offset0, offset1;
	const auto curEnd = endTime();

	/// Zeroes a range of bytes in buffers_
	const auto zeroByteRange = [&](uint32_t byteOffset, uint32_t byteCount) noexcept {
		const auto bufferCount = format_.ChannelStreamCount();
		for(uint32_t i = 0; i < bufferCount; ++i)
			std::memset(static_cast<unsigned char *>(buffers_[i]) + byteOffset,
						0,
						byteCount);
	};

	if(sampleTime > curEnd) {
		// Zero the range of samples being skipped
		offset0 = FrameByteOffset(curEnd);
		offset1 = FrameByteOffset(sampleTime);
		if(offset0 < offset1)
			zeroByteRange(offset0, offset1 - offset0);
		else {
			zeroByteRange(offset0, (capacity_ * format_.mBytesPerFrame) - offset0);
			zeroByteRange(0, offset1);
		}

		offset0 = offset1;
	} else
		offset0 = FrameByteOffset(sampleTime);

	/// Copies non-interleaved audio from _buffers to an AudioBufferList.
	const auto storeABL = [&](uint32_t dstOffset, const AudioBufferList * const _Nonnull bufferList, uint32_t srcOffset, uint32_t byteCount) noexcept {
		for(UInt32 i = 0; i < bufferList->mNumberBuffers; ++i) {
			assert(srcOffset <= bufferList->mBuffers[i].mDataByteSize);
			std::memcpy(static_cast<unsigned char *>(buffers_[i]) + dstOffset,
						static_cast<unsigned char *>(bufferList->mBuffers[i].mData) + srcOffset,
						std::min(byteCount, bufferList->mBuffers[i].mDataByteSize - srcOffset));
		}
	};

	offset1 = FrameByteOffset(endWrite);
	if(offset0 < offset1)
		storeABL(offset0, bufferList, 0, offset1 - offset0);
	else {
		const auto byteCount = (capacity_ * format_.mBytesPerFrame) - offset0;
		storeABL(offset0, bufferList, 0, byteCount);
		storeABL(0, bufferList, byteCount, offset1);
	}

	// Update the end time
	setTimeBounds(startTime(), endWrite);

	return true;
}

bool CXXCoreAudio::CARingBuffer::Read(AudioBufferList * const bufferList, uint32_t frameCount, int64_t sampleTime) noexcept
{
	if(frameCount == 0) [[unlikely]]
		return true;

	if(!bufferList || frameCount > capacity_ || sampleTime < 0) [[unlikely]]
		return false;

	auto endRead = sampleTime + static_cast<int64_t>(frameCount);

	const auto startRead0 = sampleTime;
	const auto endRead0 = endRead;

	/// Constrains start and end to valid timestamps in the buffer.
	const auto clampTimesToBounds = [&](int64_t& start, int64_t& end) noexcept -> bool {
		int64_t startTime, endTime;
		if(!GetTimeBounds(startTime, endTime)) [[unlikely]]
			return false;

		if(start > endTime || end < startTime) {
			end = start;
			return true;
		}

		start = std::max(start, startTime);
		end = std::clamp(end, start, endTime);

		return true;
	};

	if(!clampTimesToBounds(sampleTime, endRead))
		return false;

	/// Zeroes a range of bytes in @c bufferList
	const auto zeroABL = [](AudioBufferList * const _Nonnull bufferList, uint32_t byteOffset, uint32_t byteCount) noexcept {
		for(UInt32 i = 0; i < bufferList->mNumberBuffers; ++i) {
			assert(byteOffset <= bufferList->mBuffers[i].mDataByteSize);
			std::memset(static_cast<unsigned char *>(bufferList->mBuffers[i].mData) + byteOffset,
						0,
						std::min(byteCount, bufferList->mBuffers[i].mDataByteSize - byteOffset));
		}
	};

	if(sampleTime == endRead) {
		zeroABL(bufferList, 0, frameCount * format_.mBytesPerFrame);
		return true;
	}

	const auto byteSize = static_cast<uint32_t>(endRead - sampleTime) * format_.mBytesPerFrame;

	const auto destStartOffset = static_cast<uint32_t>(std::max(int64_t{0}, sampleTime - startRead0));
	const auto destStartByteOffset = destStartOffset * format_.mBytesPerFrame;
	if(destStartByteOffset > 0)
		zeroABL(bufferList, 0, std::min(frameCount * format_.mBytesPerFrame, destStartByteOffset));

	const auto destEndSize = static_cast<uint32_t>(std::max(int64_t{0}, endRead0 - endRead));
	if(destEndSize > 0)
		zeroABL(bufferList, destStartByteOffset + byteSize, destEndSize * format_.mBytesPerFrame);

	const auto byteOffset0 = FrameByteOffset(sampleTime);
	const auto byteOffset1 = FrameByteOffset(endRead);
	uint32_t byteCount;

	const auto fetchABL = [&](AudioBufferList * const _Nonnull bufferList, uint32_t dstOffset, uint32_t srcOffset, uint32_t byteCount) noexcept {
		for(UInt32 i = 0; i < bufferList->mNumberBuffers; ++i) {
			assert(dstOffset <= bufferList->mBuffers[i].mDataByteSize);
			std::memcpy(static_cast<unsigned char *>(bufferList->mBuffers[i].mData) + dstOffset,
						static_cast<unsigned char *>(buffers_[i]) + srcOffset,
						std::min(byteCount, bufferList->mBuffers[i].mDataByteSize - dstOffset));
		}
	};

	if(byteOffset0 < byteOffset1) {
		byteCount = byteOffset1 - byteOffset0;
		fetchABL(bufferList, destStartByteOffset, byteOffset0, byteCount);
	} else {
		byteCount = (capacity_ * format_.mBytesPerFrame) - byteOffset0;
		fetchABL(bufferList, destStartByteOffset, byteOffset0, byteCount);
		fetchABL(bufferList, destStartByteOffset + byteCount, 0, byteOffset1);
		byteCount += byteOffset1;
	}

	// Set the ABL buffer sizes
	for(UInt32 i = 0; i < bufferList->mNumberBuffers; ++i)
		bufferList->mBuffers[i].mDataByteSize = byteCount;

	return true;
}
