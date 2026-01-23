//
// SPDX-FileCopyrightText: 2013 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXCoreAudio
//

#import "AudioRingBuffer.hpp"

#import <cstdlib>
#import <limits>
#import <new>
#import <stdexcept>
#import <utility>

namespace {

/// Returns the number of leading 0-bits in x, starting at the most significant bit position.
template <typename T>
constexpr int clz(T x) noexcept {
    static_assert(std::is_unsigned_v<T>, "Only unsigned types supported");
    if (x == 0) {
        return sizeof(T) * CHAR_BIT;
    }
    if constexpr (sizeof(T) < sizeof(unsigned int)) {
        return __builtin_clz(x) - (sizeof(unsigned int) - sizeof(T)) * CHAR_BIT;
    } else if constexpr (sizeof(T) == sizeof(unsigned int)) {
        return __builtin_clz(x);
    } else if constexpr (sizeof(T) == sizeof(unsigned long)) {
        return __builtin_clzl(x);
    } else {
        return __builtin_clzll(x);
    }
}

/// Calculates and returns the smallest integral power of two not less than x.
/// @param x A value on the closed interval [0, 2147483648].
/// @return The smallest integral power of two not less than x.
template <typename T>
constexpr T bit_ceil(T x) noexcept {
    static_assert(std::is_unsigned_v<T>, "Only unsigned types supported");
    if (x < 2) {
        return 1;
    }
    const auto n = std::numeric_limits<T>::digits - clz(x - 1);
    assert(n != std::numeric_limits<T>::digits);
    return T{1} << n;
}

} /* namespace */

// MARK: Creation and Destruction

CXXCoreAudio::AudioRingBuffer::AudioRingBuffer(const AudioStreamBasicDescription& format, size_type minFrameCapacity) {
    if ((format.mFormatFlags & kAudioFormatFlagIsNonInterleaved) == 0 || format.mBytesPerFrame == 0 ||
        format.mChannelsPerFrame == 0) [[unlikely]] {
        throw std::invalid_argument("unsupported audio format");
    }
    if (minFrameCapacity < min_capacity || minFrameCapacity > max_capacity) [[unlikely]] {
        throw std::invalid_argument("capacity out of range");
    }
    if (!Allocate(format, minFrameCapacity)) [[unlikely]] {
        throw std::bad_alloc();
    }
}

CXXCoreAudio::AudioRingBuffer::AudioRingBuffer(AudioRingBuffer&& other) noexcept
  : buffers_{std::exchange(other.buffers_, nullptr)},
    capacity_{std::exchange(other.capacity_, 0)},
    capacityMask_{std::exchange(other.capacityMask_, 0)},
    writePosition_{other.writePosition_.exchange(0, std::memory_order_relaxed)},
    readPosition_{other.readPosition_.exchange(0, std::memory_order_relaxed)},
    format_{std::exchange(other.format_, {})} {}

CXXCoreAudio::AudioRingBuffer& CXXCoreAudio::AudioRingBuffer::operator=(AudioRingBuffer&& other) noexcept {
    if (this != &other) [[likely]] {
        std::free(buffers_);
        buffers_ = std::exchange(other.buffers_, nullptr);

        capacity_ = std::exchange(other.capacity_, 0);
        capacityMask_ = std::exchange(other.capacityMask_, 0);

        writePosition_.store(other.writePosition_.exchange(0, std::memory_order_relaxed), std::memory_order_relaxed);
        readPosition_.store(other.readPosition_.exchange(0, std::memory_order_relaxed), std::memory_order_relaxed);

        format_ = std::exchange(other.format_, {});
    }
    return *this;
}

CXXCoreAudio::AudioRingBuffer::~AudioRingBuffer() noexcept {
    std::free(buffers_);
}

// MARK: Buffer Management

bool CXXCoreAudio::AudioRingBuffer::Allocate(const AudioStreamBasicDescription& format,
                                             size_type minFrameCapacity) noexcept {
    if ((format.mFormatFlags & kAudioFormatFlagIsNonInterleaved) == 0 || format.mBytesPerFrame == 0 ||
        format.mChannelsPerFrame == 0) [[unlikely]] {
        return false;
    }
    if (minFrameCapacity < min_capacity || minFrameCapacity > max_capacity) [[unlikely]] {
        return false;
    }

    /// Values larger than this will overflow AudioBuffer.mDataByteSize
    const auto maxAudioBufferFrameCount = std::numeric_limits<UInt32>::max() / format.mBytesPerFrame;
    /// Values larger than this will exceed the maximum allocation size
    const auto maxAllocationFrameCount =
          ((std::numeric_limits<std::size_t>::max() / format.mChannelsPerFrame) - sizeof(void *)) /
          format.mBytesPerFrame;

    /// The maximum size per channel buffer in audio frames
    const auto maxChannelBufferFrameSize =
          std::min(static_cast<std::size_t>(maxAudioBufferFrameCount), maxAllocationFrameCount);

    // Round to nearest power of two
    const auto channelBufferFrameSize = bit_ceil(minFrameCapacity);
    if (channelBufferFrameSize > maxChannelBufferFrameSize) [[unlikely]] {
        return false;
    }

    Deallocate();

    const auto channelBufferByteSize = channelBufferFrameSize * format.mBytesPerFrame;
    const auto allocationSize = (channelBufferByteSize + sizeof(void *)) * format.mChannelsPerFrame;

    auto allocation = std::malloc(allocationSize);
    if (!allocation) [[unlikely]] {
        return false;
    }

    // Zero the entire allocation
    std::memset(allocation, 0, allocationSize);

    // Assign the channel buffers
    auto address = reinterpret_cast<uintptr_t>(allocation);

    buffers_ = reinterpret_cast<void **>(address);
    address += format.mChannelsPerFrame * sizeof(void *);
    for (UInt32 i = 0; i < format.mChannelsPerFrame; ++i) {
        buffers_[i] = reinterpret_cast<void *>(address);
        address += channelBufferByteSize;
    }

    capacity_ = channelBufferFrameSize;
    capacityMask_ = channelBufferFrameSize - 1;

    writePosition_.store(0, std::memory_order_relaxed);
    readPosition_.store(0, std::memory_order_relaxed);

    format_ = format;

    return true;
}

void CXXCoreAudio::AudioRingBuffer::Deallocate() noexcept {
    if (buffers_) [[likely]] {
        std::free(buffers_);
        buffers_ = nullptr;

        capacity_ = 0;
        capacityMask_ = 0;

        writePosition_.store(0, std::memory_order_relaxed);
        readPosition_.store(0, std::memory_order_relaxed);

        format_.Reset();
    }
}
