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

#import "AudioRingBuffer.hpp"

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

CXXCoreAudio::AudioRingBuffer::AudioRingBuffer(const AudioStreamBasicDescription& format, uint32_t size)
{
	if((format.mFormatFlags & kAudioFormatFlagIsNonInterleaved) == 0 || format.mBytesPerFrame == 0 || format.mChannelsPerFrame == 0)
		throw std::invalid_argument("unsupported audio format");
	if(size < 2 || size > 0x80000000)
		throw std::invalid_argument("capacity out of range");
	if(!Allocate(format, size))
		throw std::bad_alloc();
}

CXXCoreAudio::AudioRingBuffer::AudioRingBuffer(AudioRingBuffer&& other) noexcept
: buffers_{std::exchange(other.buffers_, nullptr)}, capacity_{std::exchange(other.capacity_, 0)}, capacityMask_{std::exchange(other.capacityMask_, 0)}, writePosition_{std::atomic_exchange(&other.writePosition_, 0)}, readPosition_{std::atomic_exchange(&other.readPosition_, 0)}, format_{std::exchange(other.format_, {})}
{}

CXXCoreAudio::AudioRingBuffer& CXXCoreAudio::AudioRingBuffer::operator=(AudioRingBuffer&& other) noexcept
{
	if(this != &other) {
		std::free(buffers_);
		buffers_ = std::exchange(other.buffers_, nullptr);
		capacity_ = std::exchange(other.capacity_, 0);
		capacityMask_ = std::exchange(other.capacityMask_, 0);
		writePosition_ = std::atomic_exchange(&other.writePosition_, 0);
		readPosition_ = std::atomic_exchange(&other.readPosition_, 0);
		format_ = std::exchange(other.format_, {});
	}
	return *this;
}

CXXCoreAudio::AudioRingBuffer::~AudioRingBuffer() noexcept
{
	std::free(buffers_);
}

// MARK: Buffer Management

bool CXXCoreAudio::AudioRingBuffer::Allocate(const AudioStreamBasicDescription& format, uint32_t size) noexcept
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

	readPosition_ = 0;
	writePosition_ = 0;

	format_ = format;

	return true;
}

void CXXCoreAudio::AudioRingBuffer::Deallocate() noexcept
{
	if(buffers_) {
		std::free(buffers_);
		buffers_ = nullptr;

		capacity_ = 0;
		capacityMask_ = 0;

		readPosition_ = 0;
		writePosition_ = 0;

		format_.Reset();
	}
}

void CXXCoreAudio::AudioRingBuffer::Reset() noexcept
{
	readPosition_ = 0;
	writePosition_ = 0;
}

// MARK: Buffer Information

uint32_t CXXCoreAudio::AudioRingBuffer::Capacity() const noexcept
{
	if(capacity_ == 0)
		return 0;
	return capacity_ - 1;
}

// MARK: Writing and Reading Audio

uint32_t CXXCoreAudio::AudioRingBuffer::Write(const AudioBufferList * const source, uint32_t count, bool allowPartial) noexcept
{
	if(!source || count == 0 || capacity_ == 0)
		return 0;

	const auto writePosition = writePosition_.load(std::memory_order_acquire);
	const auto readPosition = readPosition_.load(std::memory_order_acquire);

	uint32_t framesAvailable;
	if(writePosition > readPosition)
		framesAvailable = ((readPosition - writePosition + capacity_) & capacityMask_) - 1;
	else if(writePosition < readPosition)
		framesAvailable = (readPosition - writePosition) - 1;
	else
		framesAvailable = capacity_ - 1;

	if(framesAvailable == 0 || (framesAvailable < count && !allowPartial))
		return 0;

	/// Copies non-interleaved audio from an AudioBufferList to _buffers.
	const auto copyToBuffers = [&](const AudioBufferList * const _Nonnull source, uint32_t readOffset, uint32_t frameLength, uint32_t writeOffset) noexcept {
		const auto readOffsetBytes = readOffset * format_.mBytesPerFrame;
		const auto byteCount = frameLength * format_.mBytesPerFrame;
		const auto writeOffsetBytes = writeOffset * format_.mBytesPerFrame;
		for(UInt32 i = 0; i < source->mNumberBuffers; ++i) {
			assert(readOffsetBytes <= source->mBuffers[i].mDataByteSize);
			const auto dst = reinterpret_cast<uintptr_t>(buffers_[i]) + writeOffsetBytes;
			const auto src = reinterpret_cast<uintptr_t>(source->mBuffers[i].mData) + readOffsetBytes;
			const auto n = std::min(byteCount, source->mBuffers[i].mDataByteSize - readOffsetBytes);
			std::memcpy(reinterpret_cast<void *>(dst), reinterpret_cast<const void *>(src), n);
		}
	};

	const auto framesToWrite = std::min(framesAvailable, count);
	if(writePosition + framesToWrite > capacity_) {
		const auto framesAfterWritePosition = capacity_ - writePosition;
		copyToBuffers(source, 0, framesAfterWritePosition, writePosition);
		copyToBuffers(source, framesAfterWritePosition, (framesToWrite - framesAfterWritePosition), 0);
	}
	else
		copyToBuffers(source, 0, framesToWrite, writePosition);

	writePosition_.store((writePosition + framesToWrite) & capacityMask_, std::memory_order_release);

	return framesToWrite;
}

uint32_t CXXCoreAudio::AudioRingBuffer::Read(AudioBufferList * const destination, uint32_t count, bool allowPartial) noexcept
{
	if(!destination || count == 0 || capacity_ == 0)
		return 0;

	const auto writePosition = writePosition_.load(std::memory_order_acquire);
	const auto readPosition = readPosition_.load(std::memory_order_acquire);

	uint32_t framesAvailable;
	if(writePosition > readPosition)
		framesAvailable = writePosition - readPosition;
	else
		framesAvailable = (writePosition - readPosition + capacity_) & capacityMask_;

	if(framesAvailable == 0 || (framesAvailable < count && !allowPartial))
		return 0;

	/// Copies non-interleaved audio from _buffers to an AudioBufferList.
	const auto copyFromBuffers = [&](uint32_t readOffset, uint32_t frameLength, AudioBufferList * const _Nonnull destination, uint32_t writeOffset) noexcept {
		const auto readOffsetBytes = readOffset * format_.mBytesPerFrame;
		const auto byteCount = frameLength * format_.mBytesPerFrame;
		const auto writeOffsetBytes = writeOffset * format_.mBytesPerFrame;
		for(UInt32 i = 0; i < destination->mNumberBuffers; ++i) {
			assert(writeOffsetBytes <= destination->mBuffers[i].mDataByteSize);
			const auto dst = reinterpret_cast<uintptr_t>(destination->mBuffers[i].mData) + writeOffsetBytes;
			const auto src = reinterpret_cast<uintptr_t>(buffers_[i]) + readOffsetBytes;
			const auto n = std::min(byteCount, destination->mBuffers[i].mDataByteSize - writeOffsetBytes);
			std::memcpy(reinterpret_cast<void *>(dst), reinterpret_cast<const void *>(src), n);
		}
	};

	const auto framesToRead = std::min(framesAvailable, count);
	if(readPosition + framesToRead > capacity_) {
		const auto framesAfterReadPosition = capacity_ - readPosition;
		copyFromBuffers(readPosition, framesAfterReadPosition, destination, 0);
		copyFromBuffers(0, framesToRead - framesAfterReadPosition, destination, framesAfterReadPosition);
	}
	else
		copyFromBuffers(readPosition, framesToRead, destination, 0);

	readPosition_.store((readPosition + framesToRead) & capacityMask_, std::memory_order_release);

	// Set the ABL buffer sizes
	const auto byteSize = static_cast<UInt32>(framesToRead) * format_.mBytesPerFrame;
	for(UInt32 bufferIndex = 0; bufferIndex < destination->mNumberBuffers; ++bufferIndex)
		destination->mBuffers[bufferIndex].mDataByteSize = byteSize;

	return framesToRead;
}
