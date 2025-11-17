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
	if((format.mFormatFlags & kAudioFormatFlagIsNonInterleaved) == 0 || format.mBytesPerFrame == 0 || format.mChannelsPerFrame == 0)
		throw std::invalid_argument("unsupported audio format");
	if(size < 2 || size > 0x80000000)
		throw std::invalid_argument("capacity out of range");
	if(!Allocate(format, size))
		throw std::bad_alloc();
}

CXXCoreAudio::CARingBuffer::CARingBuffer(CARingBuffer&& other) noexcept
: buffers_{std::exchange(other.buffers_, nullptr)}, capacity_{std::exchange(other.capacity_, 0)}, capacityMask_{std::exchange(other.capacityMask_, 0)}, timeBoundsQueueCounter_{std::atomic_exchange(&other.timeBoundsQueueCounter_, 0)}, format_{std::exchange(other.format_, {})}
{
	for(uint32_t i = 0; i < sTimeBoundsQueueSize; ++i) {
		timeBoundsQueue_[i].startTime_ = std::exchange(other.timeBoundsQueue_[i].startTime_, 0);
		timeBoundsQueue_[i].endTime_ = std::exchange(other.timeBoundsQueue_[i].endTime_, 0);
		timeBoundsQueue_[i].updateCounter_ = std::atomic_exchange(&other.timeBoundsQueue_[i].updateCounter_, 0);
	}
}

CXXCoreAudio::CARingBuffer& CXXCoreAudio::CARingBuffer::operator=(CARingBuffer&& other) noexcept
{
	if(this != &other) {
		std::free(buffers_);
		buffers_ = std::exchange(other.buffers_, nullptr);
		capacity_ = std::exchange(other.capacity_, 0);
		capacityMask_ = std::exchange(other.capacityMask_, 0);
		for(uint32_t i = 0; i < sTimeBoundsQueueSize; ++i) {
			timeBoundsQueue_[i].startTime_ = std::exchange(other.timeBoundsQueue_[i].startTime_, 0);
			timeBoundsQueue_[i].endTime_ = std::exchange(other.timeBoundsQueue_[i].endTime_, 0);
			timeBoundsQueue_[i].updateCounter_ = std::atomic_exchange(&other.timeBoundsQueue_[i].updateCounter_, 0);
		}
		timeBoundsQueueCounter_ =  std::atomic_exchange(&other.timeBoundsQueueCounter_, 0);
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
	if((format.mFormatFlags & kAudioFormatFlagIsNonInterleaved) == 0 || format.mBytesPerFrame == 0 || format.mChannelsPerFrame == 0)
		return false;
	if(size < 2 || size > 0x80000000)
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
	if(!allocation)
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
		timeBoundsQueue_[i].updateCounter_ = 0;
	}
	timeBoundsQueueCounter_ = 0;

	return true;
}

void CXXCoreAudio::CARingBuffer::Deallocate() noexcept
{
	if(buffers_) {
		std::free(buffers_);
		buffers_ = nullptr;

		capacity_ = 0;
		capacityMask_ = 0;

		for(uint32_t i = 0; i < sTimeBoundsQueueSize; ++i) {
			timeBoundsQueue_[i].startTime_ = 0;
			timeBoundsQueue_[i].endTime_ = 0;
			timeBoundsQueue_[i].updateCounter_ = 0;
		}
		timeBoundsQueueCounter_ = 0;

		format_.Reset();
	}
}

uint32_t CXXCoreAudio::CARingBuffer::Capacity() const noexcept
{
	if(capacity_ == 0)
		return 0;
	return capacity_ - 1;
}

bool CXXCoreAudio::CARingBuffer::GetTimeBounds(int64_t& startTime, int64_t& endTime) const noexcept
{
	for(auto i = 0; i < 8; ++i) {
		const auto currentCounter = timeBoundsQueueCounter_.load(std::memory_order_acquire);
		const auto currentIndex = currentCounter & sTimeBoundsQueueMask;

		const TimeBounds * const bounds = timeBoundsQueue_ + currentIndex;
		if(const auto counter = bounds->updateCounter_.load(std::memory_order_acquire); counter == currentCounter) {
			startTime = bounds->startTime_;
			endTime = bounds->endTime_;
			return true;
		}
	}

	return false;
}

// MARK: Reading and Writing Audio

bool CXXCoreAudio::CARingBuffer::Read(AudioBufferList * const destination, uint32_t count, int64_t startRead) noexcept
{
	if(count == 0)
		return true;

	if(!destination || count > capacity_ || startRead < 0)
		return false;

	auto endRead = startRead + static_cast<int64_t>(count);

	const auto startRead0 = startRead;
	const auto endRead0 = endRead;

	/// Constrains start and end to valid timestamps in the buffer.
	const auto clampTimesToBounds = [&](int64_t& start, int64_t& end) noexcept -> bool {
		int64_t startTime, endTime;
		if(!GetTimeBounds(startTime, endTime))
			return false;

		if(start > endTime || end < startTime) {
			end = start;
			return true;
		}

		start = std::max(start, startTime);
		end = std::clamp(end, start, endTime);

		return true;
	};

	if(!clampTimesToBounds(startRead, endRead))
		return false;

	/// Zeroes a range of bytes in @c bufferList
	const auto zeroABL = [](AudioBufferList * const _Nonnull bufferList, uint32_t byteOffset, uint32_t byteCount) noexcept {
		for(UInt32 i = 0; i < bufferList->mNumberBuffers; ++i) {
			assert(byteOffset <= bufferList->mBuffers[i].mDataByteSize);
			const auto s = reinterpret_cast<uintptr_t>(bufferList->mBuffers[i].mData) + byteOffset;
			const auto n = std::min(byteCount, bufferList->mBuffers[i].mDataByteSize - byteOffset);
			std::memset(reinterpret_cast<void *>(s), 0, n);
		}
	};

	if(startRead == endRead) {
		zeroABL(destination, 0, count * format_.mBytesPerFrame);
		return true;
	}

	const auto readSize = static_cast<uint32_t>(endRead - startRead);
	const auto byteSize = static_cast<uint32_t>(endRead - startRead) * format_.mBytesPerFrame;

	const auto destStartOffset = static_cast<uint32_t>(std::max(int64_t{0}, startRead - startRead0));
	const auto destStartByteOffset = static_cast<uint32_t>(std::max(int64_t{0}, (startRead - startRead0) * format_.mBytesPerFrame));
	if(destStartByteOffset > 0)
		zeroABL(destination, 0, std::min(count * format_.mBytesPerFrame, destStartByteOffset));

	const auto destEndSize = static_cast<uint32_t>(std::max(int64_t{0}, endRead0 - endRead));
	if(destEndSize > 0)
		zeroABL(destination, destStartByteOffset + byteSize, destEndSize * format_.mBytesPerFrame);

	const auto offset0 = FrameOffset(startRead);
	const auto offset1 = FrameOffset(endRead);
	const auto byteOffset0 = FrameByteOffset(startRead);
	const auto byteOffset1 = FrameByteOffset(endRead);
	uint32_t byteCount;

	const auto fetchABL = [&](AudioBufferList * const _Nonnull bufferList, uint32_t dstOffset, uint32_t srcOffset, uint32_t byteCount) noexcept {
		for(UInt32 i = 0; i < bufferList->mNumberBuffers; ++i) {
			assert(dstOffset <= bufferList->mBuffers[i].mDataByteSize);
			const auto dst = reinterpret_cast<uintptr_t>(bufferList->mBuffers[i].mData) + dstOffset;
			const auto src = reinterpret_cast<uintptr_t>(buffers_[i]) + srcOffset;
			const auto n = std::min(byteCount, bufferList->mBuffers[i].mDataByteSize - dstOffset);
			std::memcpy(reinterpret_cast<void *>(dst), reinterpret_cast<const void *>(src), n);
		}
	};

	if(byteOffset0 < byteOffset1) {
		byteCount = byteOffset1 - byteOffset0;
		fetchABL(destination, destStartByteOffset, byteOffset0, byteCount);
	}
	else {
		byteCount = static_cast<UInt32>((capacity_ * format_.mBytesPerFrame) - byteOffset0);
		fetchABL(destination, destStartByteOffset, byteOffset0, byteCount);
		fetchABL(destination, destStartByteOffset + byteCount, 0, byteOffset1);
		byteCount += byteOffset1;
	}

	// Set the ABL buffer sizes
	for(UInt32 i = 0; i < destination->mNumberBuffers; ++i)
		destination->mBuffers[i].mDataByteSize = static_cast<UInt32>(byteCount);

	return true;
}

bool CXXCoreAudio::CARingBuffer::Write(const AudioBufferList * const source, uint32_t count, int64_t startWrite) noexcept
{
	if(count == 0)
		return true;

	if(!source || count > capacity_ || startWrite < 0)
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

		timeBoundsQueueCounter_.store(nextCounter, std::memory_order_release);
	};

	const auto endWrite = startWrite + static_cast<int64_t>(count);

	// Going backwards, throw everything out
	if(startWrite < endTime())
		setTimeBounds(startWrite, startWrite);
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
		for(uint32_t i = 0; i < bufferCount; ++i) {
			const auto s = reinterpret_cast<uintptr_t>(buffers_[i]) + byteOffset;
			std::memset(reinterpret_cast<void *>(s), 0, byteCount);
		}
	};

	if(startWrite > curEnd) {
		// Zero the range of samples being skipped
		offset0 = FrameByteOffset(curEnd);
		offset1 = FrameByteOffset(startWrite);
		if(offset0 < offset1)
			zeroByteRange(offset0, offset1 - offset0);
		else {
			zeroByteRange(offset0, (capacity_ * format_.mBytesPerFrame) - offset0);
			zeroByteRange(0, offset1);
		}

		offset0 = offset1;
	}
	else
		offset0 = FrameByteOffset(startWrite);

	/// Copies non-interleaved audio from _buffers to an AudioBufferList.
	const auto storeABL = [&](uint32_t dstOffset, const AudioBufferList * const _Nonnull bufferList, uint32_t srcOffset, uint32_t byteCount) noexcept {
		for(UInt32 i = 0; i < bufferList->mNumberBuffers; ++i) {
			assert(srcOffset <= bufferList->mBuffers[i].mDataByteSize);
			const auto dst = reinterpret_cast<uintptr_t>(buffers_[i]) + dstOffset;
			const auto src = reinterpret_cast<uintptr_t>(bufferList->mBuffers[i].mData) + srcOffset;
			const auto n = std::min(byteCount, bufferList->mBuffers[i].mDataByteSize - srcOffset);
			std::memcpy(reinterpret_cast<void *>(dst), reinterpret_cast<const void *>(src), n);
		}
	};

	offset1 = FrameByteOffset(endWrite);
	if(offset0 < offset1)
		storeABL(offset0, source, 0, offset1 - offset0);
	else {
		auto byteCount = (capacity_ * format_.mBytesPerFrame) - offset0;
		storeABL(offset0, source, 0, byteCount);
		storeABL(0, source, byteCount, offset1);
	}

	// Update the end time
	setTimeBounds(startTime(), endWrite);

	return true;
}
