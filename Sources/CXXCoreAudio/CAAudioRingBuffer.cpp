//
// Copyright © 2013-2025 Stephen F. Booth
// Part of https://github.com/sbooth/CXXCoreAudio
// MIT license
//

#import <cassert>
#import <cstdlib>
#import <limits>
#import <new>
#import <stdexcept>

#import "CAAudioRingBuffer.hpp"

namespace {

/// Copies non-interleaved audio from an AudioBufferList to a buffer array.
/// @param destination The destination buffer array.
/// @param writeOffset The byte offset in destination to begin writing.
/// @param source The source AudioBufferList.
/// @param readOffset The byte offset in source to begin reading.
/// @param byteCount The maximum number of bytes per buffer to read and write.
void CopyToBuffersFromABL(void * const _Nonnull * const _Nonnull destination, uint32_t writeOffset, const AudioBufferList * const _Nonnull source, uint32_t readOffset, uint32_t byteCount) noexcept
{
	for(UInt32 i = 0; i < source->mNumberBuffers; ++i) {
		assert(readOffset <= source->mBuffers[i].mDataByteSize);
		const auto dst = reinterpret_cast<uintptr_t>(destination[i]) + writeOffset;
		const auto src = reinterpret_cast<uintptr_t>(source->mBuffers[i].mData) + readOffset;
		const auto n = std::min(byteCount, source->mBuffers[i].mDataByteSize - readOffset);
		std::memcpy(reinterpret_cast<void *>(dst), reinterpret_cast<const void *>(src), n);
	}
}

/// Copies non-interleaved audio from a buffer array to an AudioBufferList.
/// @param destination The destination AudioBufferList.
/// @param writeOffset The byte offset in destination to begin writing.
/// @param source The source buffer array.
/// @param readOffset The byte offset in source to begin reading.
/// @param byteCount The maximum number of bytes per buffer to read and write.
void CopyToABLFromBuffers(AudioBufferList * const _Nonnull destination, uint32_t writeOffset, const void * const _Nonnull * const _Nonnull source, uint32_t readOffset, uint32_t byteCount) noexcept
{
	for(UInt32 i = 0; i < destination->mNumberBuffers; ++i) {
		assert(writeOffset <= destination->mBuffers[i].mDataByteSize);
		const auto dst = reinterpret_cast<uintptr_t>(destination->mBuffers[i].mData) + writeOffset;
		const auto src = reinterpret_cast<uintptr_t>(source[i]) + readOffset;
		const auto n = std::min(byteCount, destination->mBuffers[i].mDataByteSize - writeOffset);
		std::memcpy(reinterpret_cast<void *>(dst), reinterpret_cast<const void *>(src), n);
	}
}

/// Calculates and returns the smallest integral power of two not less than x.
/// @param x A value on the closed interval [2, 2147483648].
/// @return The smallest integral power of two not less than x.
constexpr uint32_t bit_ceil(uint32_t x) noexcept
{
#if __cplusplus >= 201402L
	assert(x > 1);
	assert(x <= ((std::numeric_limits<uint32_t>::max() / 2) + 1));
#endif
	return static_cast<uint32_t>(1 << (32 - __builtin_clz(x - 1)));
}

} /* namespace */

// MARK: Creation and Destruction

CXXCoreAudio::AudioRingBuffer::AudioRingBuffer(const CAStreamDescription& format, uint32_t size)
{
	if(format.IsInterleaved() || format.mBytesPerFrame == 0 || format.mChannelsPerFrame == 0)
		throw std::invalid_argument("unsupported audio format");
	if(size < 2 || size > 0x80000000)
		throw std::invalid_argument("capacity out of range");
	if(!Allocate(format, size))
		throw std::bad_alloc();
}

CXXCoreAudio::AudioRingBuffer::AudioRingBuffer(AudioRingBuffer&& other) noexcept
: buffers_{other.buffers_}, capacity_{other.capacity_}, capacityMask_{other.capacityMask_}, writePosition_{other.writePosition_.load(std::memory_order_acquire)}, readPosition_{other.readPosition_.load(std::memory_order_acquire)}, format_{other.format_}
{
	other.buffers_ = nullptr;

	other.capacity_ = 0;
	other.capacityMask_ = 0;

	other.writePosition_ = 0;
	other.readPosition_ = 0;

	other.format_.Reset();
}

CXXCoreAudio::AudioRingBuffer& CXXCoreAudio::AudioRingBuffer::operator=(AudioRingBuffer&& other) noexcept
{
	if(this == &other)
		return *this;

	std::free(buffers_);

	buffers_ = other.buffers_;

	capacity_ = other.capacity_;
	capacityMask_ = other.capacityMask_;

	writePosition_.store(other.writePosition_.load(std::memory_order_acquire), std::memory_order_release);
	readPosition_.store(other.readPosition_.load(std::memory_order_acquire), std::memory_order_release);

	format_ = other.format_;

	other.buffers_ = nullptr;

	other.capacity_ = 0;
	other.capacityMask_ = 0;

	other.writePosition_ = 0;
	other.readPosition_ = 0;

	other.format_.Reset();

	return *this;
}

CXXCoreAudio::AudioRingBuffer::~AudioRingBuffer() noexcept
{
	std::free(buffers_);
}

// MARK: Buffer Management

bool CXXCoreAudio::AudioRingBuffer::Allocate(const CAStreamDescription& format, uint32_t size) noexcept
{
	if(format.IsInterleaved() || format.mBytesPerFrame == 0 || format.mChannelsPerFrame == 0)
		return false;
	if(size < 2 || size > 0x80000000)
		return false;

	size = bit_ceil(size);
	if(size > (std::numeric_limits<uint32_t>::max() / format.mBytesPerFrame))
		return false;

	Deallocate();

	const auto channelBufferSize = size * format.mBytesPerFrame;
	const auto allocationSize = (channelBufferSize + sizeof(void *)) * format.mChannelsPerFrame;

	auto allocation = std::malloc(allocationSize);
	if(!allocation)
		return false;

	// Zero the entire allocation
	std::memset(allocation, 0, allocationSize);

	// Assign the buffers
	auto address = reinterpret_cast<uintptr_t>(allocation);

	buffers_ = reinterpret_cast<void **>(address);
	address += format.mChannelsPerFrame * sizeof(void *);
	for(UInt32 i = 0; i < format.mChannelsPerFrame; ++i) {
		buffers_[i] = reinterpret_cast<void *>(address);
		address += channelBufferSize;
	}

	capacity_ = size;
	capacityMask_ = size - 1;

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

uint32_t CXXCoreAudio::AudioRingBuffer::AvailableReadCount() const noexcept
{
	if(capacity_ == 0)
		return 0;

	const auto writePosition = writePosition_.load(std::memory_order_acquire);
	const auto readPosition = readPosition_.load(std::memory_order_acquire);

	if(writePosition > readPosition)
		return writePosition - readPosition;
	else
		return (writePosition - readPosition + capacity_) & capacityMask_;
}

uint32_t CXXCoreAudio::AudioRingBuffer::AvailableWriteCount() const noexcept
{
	if(capacity_ == 0)
		return 0;

	const auto writePosition = writePosition_.load(std::memory_order_acquire);
	const auto readPosition = readPosition_.load(std::memory_order_acquire);

	if(writePosition > readPosition)
		return ((readPosition - writePosition + capacity_) & capacityMask_) - 1;
	else if(writePosition < readPosition)
		return (readPosition - writePosition) - 1;
	else
		return capacity_ - 1;
}

// MARK: Reading and Writing Audio

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

	const auto framesToRead = std::min(framesAvailable, count);
	if(readPosition + framesToRead > capacity_) {
		const auto framesAfterReadPosition = capacity_ - readPosition;
		const auto bytesAfterReadPosition = framesAfterReadPosition * format_.mBytesPerFrame;
		CopyToABLFromBuffers(destination, 0, buffers_, readPosition * format_.mBytesPerFrame, bytesAfterReadPosition);
		CopyToABLFromBuffers(destination, bytesAfterReadPosition, buffers_, 0, (framesToRead - framesAfterReadPosition) * format_.mBytesPerFrame);
	}
	else
		CopyToABLFromBuffers(destination, 0, buffers_, readPosition * format_.mBytesPerFrame, framesToRead * format_.mBytesPerFrame);

	readPosition_.store((readPosition + framesToRead) & capacityMask_, std::memory_order_release);

	// Set the ABL buffer sizes
	const auto byteSize = static_cast<UInt32>(framesToRead) * format_.mBytesPerFrame;
	for(UInt32 bufferIndex = 0; bufferIndex < destination->mNumberBuffers; ++bufferIndex)
		destination->mBuffers[bufferIndex].mDataByteSize = byteSize;

	return framesToRead;
}

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

	const auto framesToWrite = std::min(framesAvailable, count);
	if(writePosition + framesToWrite > capacity_) {
		auto framesAfterWritePosition = capacity_ - writePosition;
		auto bytesAfterWritePosition = framesAfterWritePosition * format_.mBytesPerFrame;
		CopyToBuffersFromABL(buffers_, writePosition * format_.mBytesPerFrame, source, 0, bytesAfterWritePosition);
		CopyToBuffersFromABL(buffers_, 0, source, bytesAfterWritePosition, (framesToWrite - framesAfterWritePosition) * format_.mBytesPerFrame);
	}
	else
		CopyToBuffersFromABL(buffers_, writePosition * format_.mBytesPerFrame, source, 0, framesToWrite * format_.mBytesPerFrame);

	writePosition_.store((writePosition + framesToWrite) & capacityMask_, std::memory_order_release);

	return framesToWrite;
}
