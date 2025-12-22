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

/// Copies non-interleaved audio to a buffer array from an AudioBufferList struct.
/// @param dst The destination audio buffers.
/// @param dstOffset The byte offset to begin writing.
/// @param src The source AudioBufferList.
/// @param srcOffset The byte offset to begin reading.
/// @param byteCount The number of bytes per non-interleaved buffer to read and write.
void CopyToBuffersFromAudioBufferList(void * const _Nonnull * const _Nonnull dst, std::size_t dstOffset, const AudioBufferList * const _Nonnull src, std::size_t srcOffset, std::size_t byteCount) noexcept
{
	for(UInt32 i = 0; i < src->mNumberBuffers; ++i) {
		assert(srcOffset + byteCount <= src->mBuffers[i].mDataByteSize);
		std::memcpy(static_cast<uint8_t *>(dst[i]) + dstOffset,
					static_cast<const uint8_t *>(src->mBuffers[i].mData) + srcOffset,
					byteCount);
	}
}

/// Copies non-interleaved audio to an AudioBufferList struct from a buffer array.
/// @param dst The destination AudioBufferList.
/// @param dstOffset The byte offset to begin writing.
/// @param src The source audio buffers.
/// @param srcOffset The byte offset to begin reading.
/// @param byteCount The number of bytes per non-interleaved buffer to read and write.
void CopyToAudioBufferListFromBuffers(AudioBufferList * const _Nonnull dst, std::size_t dstOffset, const void * const _Nonnull * const _Nonnull src, std::size_t srcOffset, std::size_t byteCount) noexcept
{
	for(UInt32 i = 0; i < dst->mNumberBuffers; ++i) {
		assert(dstOffset + byteCount <= dst->mBuffers[i].mDataByteSize);
		std::memcpy(static_cast<uint8_t *>(dst->mBuffers[i].mData) + dstOffset,
					static_cast<const uint8_t *>(src[i]) + srcOffset,
					byteCount);
	}
}

/// Zeroes the bytes in an AudioBufferList struct.
/// @param abl The destination AudioBufferList.
void ZeroAudioBufferList(AudioBufferList * const _Nonnull abl) noexcept
{
	for(UInt32 i = 0; i < abl->mNumberBuffers; ++i)
		std::memset(abl->mBuffers[i].mData, 0, abl->mBuffers[i].mDataByteSize);
}

/// Zeroes a range of bytes in an AudioBufferList struct.
/// @param abl The destination AudioBufferList.
/// @param byteOffset The byte offset to begin writing zeroes.
/// @param byteCount The number of bytes to set to zero.
void ZeroAudioBufferList(AudioBufferList * const _Nonnull abl, std::size_t byteOffset, std::size_t byteCount) noexcept
{
	for(UInt32 i = 0; i < abl->mNumberBuffers; ++i) {
		assert(byteOffset + byteCount <= abl->mBuffers[i].mDataByteSize);
		std::memset(static_cast<uint8_t *>(abl->mBuffers[i].mData) + byteOffset, 0, byteCount);
	}
}

/// Returns the number of leading 0-bits in x, starting at the most significant bit position.
template <typename T>
constexpr int clz(T x) noexcept
{
	static_assert(std::is_unsigned_v<T>, "Only unsigned types supported");
	if(x == 0)
		return sizeof(T) * CHAR_BIT;
	if constexpr (sizeof(T) < sizeof(unsigned int))
		return __builtin_clz(x) - (sizeof(unsigned int) - sizeof(T)) * CHAR_BIT;
	else if constexpr (sizeof(T) == sizeof(unsigned int))
		return __builtin_clz(x);
	else if constexpr (sizeof(T) == sizeof(unsigned long))
		return __builtin_clzl(x);
	else
		return __builtin_clzll(x);
}

/// Calculates and returns the smallest integral power of two not less than x.
/// @param x A value on the closed interval [0, 2147483648].
/// @return The smallest integral power of two not less than x.
template <typename T>
constexpr T bit_ceil(T x) noexcept
{
	static_assert(std::is_unsigned_v<T>, "Only unsigned types supported");
	if(x < 2)
		return 1;
	const auto n = std::numeric_limits<T>::digits - clz(x - 1);
	assert(n != std::numeric_limits<T>::digits);
	return T{1} << n;
}

} /* namespace */

// MARK: Creation and Destruction

CXXCoreAudio::AudioRingBuffer::AudioRingBuffer(const AudioStreamBasicDescription& format, size_type minFrameCapacity)
{
	if((format.mFormatFlags & kAudioFormatFlagIsNonInterleaved) == 0 || format.mBytesPerFrame == 0 || format.mChannelsPerFrame == 0) [[unlikely]]
		throw std::invalid_argument("unsupported audio format");
	if(minFrameCapacity < min_capacity || minFrameCapacity > max_capacity) [[unlikely]]
		throw std::invalid_argument("capacity out of range");
	if(!Allocate(format, minFrameCapacity)) [[unlikely]]
		throw std::bad_alloc();
}

CXXCoreAudio::AudioRingBuffer::AudioRingBuffer(AudioRingBuffer&& other) noexcept
: buffers_{std::exchange(other.buffers_, nullptr)}, capacity_{std::exchange(other.capacity_, 0)}, capacityMask_{std::exchange(other.capacityMask_, 0)}, writePosition_{other.writePosition_.exchange(0, std::memory_order_relaxed)}, readPosition_{other.readPosition_.exchange(0, std::memory_order_relaxed)}, epoch_{other.epoch_.exchange(0, std::memory_order_relaxed)}, readEpoch_{other.readEpoch_.exchange(0, std::memory_order_relaxed)}, format_{std::exchange(other.format_, {})}
{}

CXXCoreAudio::AudioRingBuffer& CXXCoreAudio::AudioRingBuffer::operator=(AudioRingBuffer&& other) noexcept
{
	if(this != &other) [[likely]] {
		std::free(buffers_);
		buffers_ = std::exchange(other.buffers_, nullptr);

		capacity_ = std::exchange(other.capacity_, 0);
		capacityMask_ = std::exchange(other.capacityMask_, 0);

		writePosition_.store(other.writePosition_.exchange(0, std::memory_order_relaxed), std::memory_order_relaxed);
		readPosition_.store(other.readPosition_.exchange(0, std::memory_order_relaxed), std::memory_order_relaxed);

		epoch_.store(other.epoch_.exchange(0, std::memory_order_relaxed), std::memory_order_relaxed);
		readEpoch_.store(other.readEpoch_.exchange(0, std::memory_order_relaxed), std::memory_order_relaxed);

		format_ = std::exchange(other.format_, {});
	}
	return *this;
}

CXXCoreAudio::AudioRingBuffer::~AudioRingBuffer() noexcept
{
	std::free(buffers_);
}

// MARK: Buffer Management

bool CXXCoreAudio::AudioRingBuffer::Allocate(const AudioStreamBasicDescription& format, size_type minFrameCapacity) noexcept
{
	if((format.mFormatFlags & kAudioFormatFlagIsNonInterleaved) == 0 || format.mBytesPerFrame == 0 || format.mChannelsPerFrame == 0) [[unlikely]]
		return false;
	if(minFrameCapacity < min_capacity || minFrameCapacity > max_capacity) [[unlikely]]
		return false;

	/// Values larger than this will overflow AudioBuffer.mDataByteSize
	const auto maxAudioBufferFrameCount = std::numeric_limits<UInt32>::max() / format.mBytesPerFrame;
	/// Values larger than this will exceed the maximum allocation size
	const auto maxAllocationFrameCount = ((std::numeric_limits<std::size_t>::max() / format.mChannelsPerFrame) - sizeof(void *)) / format.mBytesPerFrame;

	/// The maximum size per channel buffer in audio frames
	const auto maxChannelBufferFrameSize = std::min(static_cast<std::size_t>(maxAudioBufferFrameCount), maxAllocationFrameCount);

	// Round to nearest power of two
	const auto channelBufferFrameSize = bit_ceil(minFrameCapacity);
	if(channelBufferFrameSize > maxChannelBufferFrameSize) [[unlikely]]
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

	writePosition_.store(0, std::memory_order_relaxed);
	readPosition_.store(0, std::memory_order_relaxed);

	epoch_.store(0, std::memory_order_relaxed);
	readEpoch_.store(0, std::memory_order_relaxed);

	format_ = format;

	return true;
}

void CXXCoreAudio::AudioRingBuffer::Deallocate() noexcept
{
	if(buffers_) [[likely]] {
		std::free(buffers_);
		buffers_ = nullptr;

		capacity_ = 0;
		capacityMask_ = 0;

		writePosition_.store(0, std::memory_order_relaxed);
		readPosition_.store(0, std::memory_order_relaxed);

		epoch_.store(0, std::memory_order_relaxed);
		readEpoch_.store(0, std::memory_order_relaxed);

		format_.Reset();
	}
}

// MARK: Buffer Usage

CXXCoreAudio::AudioRingBuffer::size_type CXXCoreAudio::AudioRingBuffer::FreeSpace() const noexcept
{
	const auto writePos = writePosition_.load(std::memory_order_relaxed);
	const auto readPos = readPosition_.load(std::memory_order_acquire);
	return capacity_ - (writePos - readPos);
}

CXXCoreAudio::AudioRingBuffer::size_type CXXCoreAudio::AudioRingBuffer::AvailableFrames() const noexcept
{
	const auto writePos = writePosition_.load(std::memory_order_acquire);
	const auto readPos = readPosition_.load(std::memory_order_relaxed);
	return writePos - readPos;
}

bool CXXCoreAudio::AudioRingBuffer::IsEmpty() const noexcept
{
	const auto writePos = writePosition_.load(std::memory_order_acquire);
	const auto readPos = readPosition_.load(std::memory_order_acquire);
	return writePos == readPos;
}

bool CXXCoreAudio::AudioRingBuffer::IsFull() const noexcept
{
	const auto writePos = writePosition_.load(std::memory_order_acquire);
	const auto readPos = readPosition_.load(std::memory_order_acquire);
	return (writePos - readPos) == capacity_;
}

// MARK: Writing and Reading Audio

CXXCoreAudio::AudioRingBuffer::size_type CXXCoreAudio::AudioRingBuffer::Write(const AudioBufferList * const bufferList, size_type frameCount) noexcept
{
	if(!bufferList || frameCount == 0 || capacity_ == 0) [[unlikely]]
		return 0;

	const auto writePos = writePosition_.load(std::memory_order_relaxed);
	const auto readPos = readPosition_.load(std::memory_order_acquire);

	const auto framesUsed = writePos - readPos;
	const auto framesFree = capacity_ - framesUsed;
	if(framesFree == 0) [[unlikely]]
		return 0;

	const auto framesToWrite = std::min(framesFree, frameCount);

	const auto writeIndex = writePos & capacityMask_;
	const auto framesToEnd = capacity_ - writeIndex;
	if(framesToWrite <= framesToEnd) [[likely]]
		CopyToBuffersFromAudioBufferList(buffers_, writeIndex * format_.mBytesPerFrame, bufferList, 0, framesToWrite * format_.mBytesPerFrame);
	else [[unlikely]] {
		const auto bytesToEnd = framesToEnd * format_.mBytesPerFrame;
		CopyToBuffersFromAudioBufferList(buffers_, writeIndex * format_.mBytesPerFrame, bufferList, 0, bytesToEnd);
		CopyToBuffersFromAudioBufferList(buffers_, 0, bufferList, bytesToEnd, (framesToWrite - framesToEnd) * format_.mBytesPerFrame);
	}

	writePosition_.store(writePos + framesToWrite, std::memory_order_release);

	return framesToWrite;
}

CXXCoreAudio::AudioRingBuffer::size_type CXXCoreAudio::AudioRingBuffer::Read(AudioBufferList * const bufferList, size_type frameCount) noexcept
{
	if(!bufferList || frameCount == 0 || capacity_ == 0) [[unlikely]]
		return 0;

	const auto currentEpoch = epoch_.load(std::memory_order_acquire);
	const auto localEpoch = readEpoch_.load(std::memory_order_relaxed);

	// If the epoch changed resynchronize with the producer
	if(currentEpoch != localEpoch) [[unlikely]] {
		readEpoch_.store(currentEpoch, std::memory_order_relaxed);

		const auto writePos = writePosition_.load(std::memory_order_acquire);
		readPosition_.store(writePos, std::memory_order_release);

		ZeroAudioBufferList(bufferList);
		return 0;
	}

	const auto writePos = writePosition_.load(std::memory_order_acquire);
	const auto readPos = readPosition_.load(std::memory_order_relaxed);

	if(writePos <= readPos) [[unlikely]] {
		ZeroAudioBufferList(bufferList);
		return 0;
	}

	const auto availableFrames = writePos - readPos;
	const auto framesToRead = std::min(availableFrames, frameCount);

	const auto readIndex = readPos & capacityMask_;
	const auto framesToEnd = capacity_ - readIndex;
	if(framesToRead <= framesToEnd) [[likely]]
		CopyToAudioBufferListFromBuffers(bufferList, 0, buffers_, readIndex * format_.mBytesPerFrame, framesToRead * format_.mBytesPerFrame);
	else [[unlikely]] {
		const auto bytesToEnd = framesToEnd * format_.mBytesPerFrame;
		CopyToAudioBufferListFromBuffers(bufferList, 0, buffers_, readIndex * format_.mBytesPerFrame, bytesToEnd);
		CopyToAudioBufferListFromBuffers(bufferList, bytesToEnd, buffers_, 0, (framesToRead - framesToEnd) * format_.mBytesPerFrame);
	}

	readPosition_.store(readPos + framesToRead, std::memory_order_release);

	// Fill remainder with silence if fewer than requested frames read
	if(framesToRead != frameCount)
		ZeroAudioBufferList(bufferList, framesToRead * format_.mBytesPerFrame, (frameCount - framesToRead) * format_.mBytesPerFrame);

	return framesToRead;
}

// MARK: Discarding Audio

CXXCoreAudio::AudioRingBuffer::size_type CXXCoreAudio::AudioRingBuffer::Skip(size_type frameCount) noexcept
{
	if(frameCount == 0 || capacity_ == 0) [[unlikely]]
		return 0;

	const auto currentEpoch = epoch_.load(std::memory_order_acquire);
	const auto localEpoch = readEpoch_.load(std::memory_order_relaxed);

	if(currentEpoch != localEpoch) [[unlikely]] {
		readEpoch_.store(currentEpoch, std::memory_order_relaxed);

		const auto writePos = writePosition_.load(std::memory_order_acquire);
		readPosition_.store(writePos, std::memory_order_release);

		return 0;
	}

	const auto writePos = writePosition_.load(std::memory_order_acquire);
	const auto readPos = readPosition_.load(std::memory_order_relaxed);

	if(writePos <= readPos) [[unlikely]]
		return 0;

	const auto availableFrames = writePos - readPos;
	const auto framesToSkip = std::min(availableFrames, frameCount);

	readPosition_.store(readPos + framesToSkip, std::memory_order_release);

	return framesToSkip;
}

void CXXCoreAudio::AudioRingBuffer::Drain() noexcept
{
	const auto currentEpoch = epoch_.load(std::memory_order_acquire);
	const auto localEpoch = readEpoch_.load(std::memory_order_relaxed);

	if(currentEpoch != localEpoch) [[unlikely]]
		readEpoch_.store(currentEpoch, std::memory_order_relaxed);

	const auto writePos = writePosition_.load(std::memory_order_acquire);
	readPosition_.store(writePos, std::memory_order_release);
}

void CXXCoreAudio::AudioRingBuffer::Clear() noexcept
{
	epoch_.fetch_add(1, std::memory_order_release);
}
