//
// SPDX-FileCopyrightText: 2013 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXCoreAudio
//

#pragma once

#include <CXXCoreAudio/CAStreamDescription.hpp>

#include <CoreAudioTypes/CoreAudioTypes.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <limits>

namespace CXXCoreAudio {

/// A lock-free SPSC audio ring buffer supporting non-interleaved audio.
///
/// This class is thread safe when used with a single producer and a single consumer.
class AudioRingBuffer final {
  public:
    /// Unsigned integer type.
    using size_type = std::size_t;
    /// Atomic unsigned integer type.
    using atomic_size_type = std::atomic<size_type>;

    /// The minimum supported buffer capacity in audio frames.
    static constexpr size_type min_capacity = size_type{2};
    /// The maximum supported buffer capacity in audio frames.
    static constexpr size_type max_capacity = size_type{1} << (std::numeric_limits<size_type>::digits - 1);

    // MARK: Creation and Destruction

    /// Creates an empty ring buffer.
    /// @note ``Allocate`` must be called before the object may be used.
    AudioRingBuffer() noexcept = default;

    /// Creates a ring buffer with the specified format and minimum audio frame capacity.
    ///
    /// The actual buffer capacity will be the smallest integral power of two that is not less than the specified
    /// minimum capacity.
    /// @note Only non-interleaved formats are supported.
    /// @param format The format of the audio that will be written to and read from the buffer.
    /// @param minFrameCapacity The desired minimum capacity in audio frames.
    /// @throw std::bad_alloc if memory could not be allocated or std::invalid_argument if the buffer capacity is not
    /// supported.
    AudioRingBuffer(const AudioStreamBasicDescription& format, size_type minFrameCapacity);

    // This class is non-copyable
    AudioRingBuffer(const AudioRingBuffer&) = delete;

    /// Creates a ring buffer by moving the contents of another ring buffer.
    /// @note This method is not thread safe for the ring buffer being moved.
    /// @param other The ring buffer to move.
    AudioRingBuffer(AudioRingBuffer&& other) noexcept;

    // This class is non-assignable
    AudioRingBuffer& operator=(const AudioRingBuffer&) = delete;

    /// Moves the contents of another ring buffer into this ring buffer.
    /// @note This method is not thread safe.
    /// @param other The ring buffer to move.
    AudioRingBuffer& operator=(AudioRingBuffer&& other) noexcept;

    /// Destroys the ring buffer and releases all associated resources.
    ~AudioRingBuffer() noexcept;

    // MARK: Buffer Management

    /// Allocates space for audio data of the specified format.
    ///
    /// The actual buffer capacity will be the smallest integral power of two that is not less than the specified
    /// minimum capacity.
    /// @note Only non-interleaved formats are supported.
    /// @note This method is not thread safe.
    /// @param format The format of the audio that will be written to and read from this buffer.
    /// @param minFrameCapacity The desired minimum capacity in audio frames.
    /// @return true on success, false if memory could not be allocated, the audio format is not supported, or the
    /// buffer capacity is not supported.
    bool Allocate(const AudioStreamBasicDescription& format, size_type minFrameCapacity) noexcept;

    /// Frees any space allocated for audio data.
    /// @note This method is not thread safe.
    void Deallocate() noexcept;

    /// Returns true if the buffer has allocated space for audio data.
    [[nodiscard]] explicit operator bool() const noexcept;

    // MARK: Buffer Information

    /// Returns the format of the audio stored in the buffer.
    /// @note This method is safe to call from both producer and consumer.
    /// @return The audio format of the buffer.
    [[nodiscard]] const CAStreamDescription& Format() const noexcept;

    /// Returns the capacity of the buffer.
    /// @note This method is safe to call from both producer and consumer.
    /// @return The buffer capacity in audio frames.
    [[nodiscard]] size_type Capacity() const noexcept;

    // MARK: Buffer Usage

    /// Returns the amount of free space in the buffer.
    /// @note The result of this method is only accurate when called from the producer.
    /// @return The number of audio frames of free space available for writing.
    [[nodiscard]] size_type FreeSpace() const noexcept;

    /// Returns true if the buffer is full.
    /// @note The result of this method is only accurate when called from the producer.
    /// @return true if the buffer is full.
    [[nodiscard]] bool IsFull() const noexcept;

    /// Returns the amount of audio in the buffer.
    /// @note The result of this method is only accurate when called from the consumer.
    /// @return The number of audio frames available for reading.
    [[nodiscard]] size_type AvailableFrames() const noexcept;

    /// Returns true if the buffer is empty.
    /// @note The result of this method is only accurate when called from the consumer.
    /// @return true if the buffer contains no data.
    [[nodiscard]] bool IsEmpty() const noexcept;

    // MARK: Writing and Reading Audio

    /// Writes audio and advances the write position.
    /// @note This method is only safe to call from the producer.
    /// @param bufferList An audio buffer list containing the data to copy.
    /// @param frameCount The desired number of audio frames to write.
    /// @return The number of audio frames actually written.
    size_type Write(const AudioBufferList *const _Nonnull bufferList, size_type frameCount) noexcept;

    /// Reads audio and advances the read position.
    ///
    /// If fewer than the requested number of frames are available the remainder of the audio buffer list will be set to
    /// zero.
    /// @note This method is only safe to call from the consumer.
    /// @param bufferList An audio buffer list to receive the data.
    /// @param frameCount The desired number of audio frames to read.
    /// @return The number of audio frames actually read.
    size_type Read(AudioBufferList *const _Nonnull bufferList, size_type frameCount) noexcept;

    // MARK: Discarding Audio

    /// Skips audio and advances the read position.
    /// @note This method is only safe to call from the consumer.
    /// @param frameCount The desired number of audio frames to skip.
    /// @return The number of audio frames actually skipped.
    size_type Skip(size_type frameCount) noexcept;

    /// Advances the read position to the write position, emptying the buffer.
    /// @note This method is only safe to call from the consumer.
    /// @return The number of audio frames discarded.
    size_type Drain() noexcept;

  private:
    /// The memory buffers holding the data, consisting of channel pointers and buffers allocated in one chunk.
    void *_Nonnull *_Nullable buffers_{nullptr};

    /// The per-channel capacity of ``buffers_`` in audio frames.
    size_type capacity_{0};
    /// The per-channel capacity of ``buffers_`` in audio frames minus one.
    size_type capacityMask_{0};

    /// The free-running write location.
	alignas(std::hardware_destructive_interference_size)
	atomic_size_type writePosition_{0};
	/// The free-running read location.
	alignas(std::hardware_destructive_interference_size)
	atomic_size_type readPosition_{0};

	static_assert(atomic_size_type::is_always_lock_free, "Lock-free atomic_size_type required");
	static_assert(std::hardware_destructive_interference_size >= alignof(atomic_size_type));

    /// The format of the audio this buffer contains.
    CAStreamDescription format_{};
};

// MARK: - Implementation -

// MARK: Buffer Management

inline AudioRingBuffer::operator bool() const noexcept {
    return buffers_ != nullptr;
}

// MARK: Buffer Information

inline const CAStreamDescription& AudioRingBuffer::Format() const noexcept {
    return format_;
}

inline AudioRingBuffer::size_type AudioRingBuffer::Capacity() const noexcept {
    return capacity_;
}

// MARK: Buffer Usage

inline AudioRingBuffer::size_type AudioRingBuffer::FreeSpace() const noexcept {
    const auto writePos = writePosition_.load(std::memory_order_relaxed);
    const auto readPos = readPosition_.load(std::memory_order_acquire);
    return capacity_ - (writePos - readPos);
}

inline bool AudioRingBuffer::IsFull() const noexcept {
    const auto writePos = writePosition_.load(std::memory_order_relaxed);
    const auto readPos = readPosition_.load(std::memory_order_acquire);
    return (writePos - readPos) == capacity_;
}

inline AudioRingBuffer::size_type AudioRingBuffer::AvailableFrames() const noexcept {
    const auto writePos = writePosition_.load(std::memory_order_acquire);
    const auto readPos = readPosition_.load(std::memory_order_relaxed);
    return writePos - readPos;
}

inline bool AudioRingBuffer::IsEmpty() const noexcept {
    const auto writePos = writePosition_.load(std::memory_order_acquire);
    const auto readPos = readPosition_.load(std::memory_order_relaxed);
    return writePos == readPos;
}

// MARK: Writing and Reading Audio

inline CXXCoreAudio::AudioRingBuffer::size_type
CXXCoreAudio::AudioRingBuffer::Write(const AudioBufferList *const _Nonnull bufferList, size_type frameCount) noexcept {
    if (!bufferList || frameCount == 0 || capacity_ == 0) [[unlikely]] {
        return 0;
    }

    const auto writePos = writePosition_.load(std::memory_order_relaxed);
    const auto readPos = readPosition_.load(std::memory_order_acquire);

    const auto framesUsed = writePos - readPos;
    const auto framesFree = capacity_ - framesUsed;
    if (framesFree == 0) [[unlikely]] {
        return 0;
    }

    /// Copies non-interleaved audio to a buffer array from an AudioBufferList struct.
    const auto copyToBuffersFromAudioBufferList = [](void *const _Nonnull *const _Nonnull dst, std::size_t dstOffset,
                                                     const AudioBufferList *const _Nonnull src, std::size_t srcOffset,
                                                     std::size_t byteCount) noexcept {
        for (UInt32 i = 0; i < src->mNumberBuffers; ++i) {
            assert(srcOffset + byteCount <= src->mBuffers[i].mDataByteSize);
            std::memcpy(static_cast<unsigned char *>(dst[i]) + dstOffset,
                        static_cast<const unsigned char *>(src->mBuffers[i].mData) + srcOffset, byteCount);
        }
    };

    const auto framesToWrite = std::min(framesFree, frameCount);

    const auto writeIndex = writePos & capacityMask_;
    const auto framesToEnd = capacity_ - writeIndex;
    if (framesToWrite <= framesToEnd) [[likely]] {
        copyToBuffersFromAudioBufferList(buffers_, writeIndex * format_.mBytesPerFrame, bufferList, 0,
                                         framesToWrite * format_.mBytesPerFrame);
    } else [[unlikely]] {
        const auto bytesToEnd = framesToEnd * format_.mBytesPerFrame;
        copyToBuffersFromAudioBufferList(buffers_, writeIndex * format_.mBytesPerFrame, bufferList, 0, bytesToEnd);
        copyToBuffersFromAudioBufferList(buffers_, 0, bufferList, bytesToEnd,
                                         (framesToWrite - framesToEnd) * format_.mBytesPerFrame);
    }

    writePosition_.store(writePos + framesToWrite, std::memory_order_release);

    return framesToWrite;
}

inline CXXCoreAudio::AudioRingBuffer::size_type
CXXCoreAudio::AudioRingBuffer::Read(AudioBufferList *const _Nonnull bufferList, size_type frameCount) noexcept {
    if (!bufferList || frameCount == 0 || capacity_ == 0) [[unlikely]] {
        return 0;
    }

    const auto writePos = writePosition_.load(std::memory_order_acquire);
    const auto readPos = readPosition_.load(std::memory_order_relaxed);

    const auto framesAvailable = writePos - readPos;
    if (framesAvailable == 0) [[unlikely]] {
        for (UInt32 i = 0; i < bufferList->mNumberBuffers; ++i) {
            std::memset(bufferList->mBuffers[i].mData, 0, bufferList->mBuffers[i].mDataByteSize);
        }
        return 0;
    }

    /// Copies non-interleaved audio to an AudioBufferList struct from a buffer array.
    const auto copyToAudioBufferListFromBuffers = [](AudioBufferList *const _Nonnull dst, std::size_t dstOffset,
                                                     const void *const _Nonnull *const _Nonnull src,
                                                     std::size_t srcOffset, std::size_t byteCount) noexcept {
        for (UInt32 i = 0; i < dst->mNumberBuffers; ++i) {
            assert(dstOffset + byteCount <= dst->mBuffers[i].mDataByteSize);
            std::memcpy(static_cast<unsigned char *>(dst->mBuffers[i].mData) + dstOffset,
                        static_cast<const unsigned char *>(src[i]) + srcOffset, byteCount);
        }
    };

    const auto framesToRead = std::min(framesAvailable, frameCount);

    const auto readIndex = readPos & capacityMask_;
    const auto framesToEnd = capacity_ - readIndex;
    if (framesToRead <= framesToEnd) [[likely]] {
        copyToAudioBufferListFromBuffers(bufferList, 0, buffers_, readIndex * format_.mBytesPerFrame,
                                         framesToRead * format_.mBytesPerFrame);
    } else [[unlikely]] {
        const auto bytesToEnd = framesToEnd * format_.mBytesPerFrame;
        copyToAudioBufferListFromBuffers(bufferList, 0, buffers_, readIndex * format_.mBytesPerFrame, bytesToEnd);
        copyToAudioBufferListFromBuffers(bufferList, bytesToEnd, buffers_, 0,
                                         (framesToRead - framesToEnd) * format_.mBytesPerFrame);
    }

    readPosition_.store(readPos + framesToRead, std::memory_order_release);

    // Fill remainder with silence if fewer than requested frames read
    if (framesToRead != frameCount) {
        const auto byteOffset = framesToRead * format_.mBytesPerFrame;
        const auto byteCount = (frameCount - framesToRead) * format_.mBytesPerFrame;
        for (UInt32 i = 0; i < bufferList->mNumberBuffers; ++i) {
            assert(byteOffset + byteCount <= bufferList->mBuffers[i].mDataByteSize);
            std::memset(static_cast<unsigned char *>(bufferList->mBuffers[i].mData) + byteOffset, 0, byteCount);
        }
    }

    return framesToRead;
}

// MARK: Discarding Audio

inline CXXCoreAudio::AudioRingBuffer::size_type CXXCoreAudio::AudioRingBuffer::Skip(size_type frameCount) noexcept {
    if (frameCount == 0 || capacity_ == 0) [[unlikely]] {
        return 0;
    }

    const auto writePos = writePosition_.load(std::memory_order_acquire);
    const auto readPos = readPosition_.load(std::memory_order_relaxed);

    const auto framesAvailable = writePos - readPos;
    if (framesAvailable == 0) [[unlikely]] {
        return 0;
    }

    const auto framesToSkip = std::min(framesAvailable, frameCount);

    readPosition_.store(readPos + framesToSkip, std::memory_order_release);

    return framesToSkip;
}

inline AudioRingBuffer::size_type AudioRingBuffer::Drain() noexcept {
    if (capacity_ == 0) [[unlikely]] {
        return 0;
    }

    const auto writePos = writePosition_.load(std::memory_order_acquire);
    const auto readPos = readPosition_.load(std::memory_order_relaxed);

    const auto framesAvailable = writePos - readPos;
    if (framesAvailable == 0) [[unlikely]] {
        return 0;
    }

    readPosition_.store(writePos, std::memory_order_release);
    return framesAvailable;
}

} /* namespace CXXCoreAudio */
