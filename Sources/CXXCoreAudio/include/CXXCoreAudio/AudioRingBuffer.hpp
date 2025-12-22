//
// Copyright © 2013-2025 Stephen F. Booth
// Part of https://github.com/sbooth/CXXCoreAudio
// MIT license
//

#pragma once

#import <atomic>
#import <cstddef>
#import <limits>

#import <CoreAudioTypes/CoreAudioTypes.h>

#import <CXXCoreAudio/CAStreamDescription.hpp>

namespace CXXCoreAudio {

/// A lock-free SPSC audio ring buffer supporting non-interleaved audio.
///
/// This class is thread safe when used with a single producer and a single consumer.
class AudioRingBuffer final {
public:
	/// Unsigned integer type.
	using size_type = std::size_t;

	/// The minimum supported ring buffer capacity in audio frames.
	static constexpr size_type min_capacity = size_type{2};
	/// The maximum supported ring buffer capacity in audio frames.
	static constexpr size_type max_capacity = size_type{1} << (std::numeric_limits<size_type>::digits - 1);

	// MARK: Creation and Destruction

	/// Creates an empty ring buffer.
	/// @note ``Allocate`` must be called before the object may be used.
	AudioRingBuffer() noexcept = default;

	/// Creates a ring buffer with the specified format and minimum audio frame capacity.
	///
	/// The actual ring buffer capacity will be the smallest integral power of two that is not less than the specified minimum capacity.
	/// @note Only non-interleaved formats are supported.
	/// @param format The format of the audio that will be written to and read from the buffer.
	/// @param minFrameCapacity The desired minimum capacity in audio frames.
	/// @throw std::bad_alloc if memory could not be allocated or std::invalid_argument if the buffer capacity is not supported.
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
	/// The actual ring buffer capacity will be the smallest integral power of two that is not less than the specified minimum capacity.
	/// @note Only non-interleaved formats are supported.
	/// @note This method is not thread safe.
	/// @param format The format of the audio that will be written to and read from this buffer.
	/// @param minFrameCapacity The desired minimum capacity in audio frames.
	/// @return true on success, false if memory could not be allocated, the audio format is not supported, or the buffer capacity is not supported.
	bool Allocate(const AudioStreamBasicDescription& format, size_type minFrameCapacity) noexcept;

	/// Frees any space allocated for audio data.
	/// @note This method is not thread safe.
	void Deallocate() noexcept;

	// MARK: Buffer Information

	/// Returns the format of the audio stored in the ring buffer.
	/// @note This method is thread safe.
	/// @return The audio format of the ring buffer.
	[[nodiscard]] const CAStreamDescription& Format() const noexcept
	{
		return format_;
	}

	/// Returns the capacity of the ring buffer.
	/// @note This method is thread safe.
	/// @return The ring buffer capacity in audio frames.
	[[nodiscard]] size_type Capacity() const noexcept
	{
		return capacity_;
	}

	// MARK: Buffer Usage

	/// Returns the amount of free space in the buffer.
	/// @note This method is only safe to call from the producer.
	/// @return The number of audio frames of free space available for writing.
	[[nodiscard]] size_type FreeSpace() const noexcept;

	/// Returns the amount of audio in the buffer.
	/// @note This method is only safe to call from the consumer.
	/// @return The number of audio frames available for reading.
	[[nodiscard]] size_type AvailableFrames() const noexcept;

	/// Returns true if the ring buffer is empty.
	/// @note This method is thread safe.
	/// @return true if the buffer contains no data.
	[[nodiscard]] bool IsEmpty() const noexcept;

	/// Returns true if the ring buffer is full.
	/// @note This method is thread safe.
	/// @return true if the buffer is full.
	[[nodiscard]] bool IsFull() const noexcept;

	// MARK: Writing and Reading Audio

	/// Writes audio and advances the write position.
	/// @note This method is only safe to call from the producer.
	/// @param bufferList An audio buffer list containing the data to copy.
	/// @param frameCount The desired number of audio frames to write.
	/// @return The number of audio frames actually written.
	size_type Write(const AudioBufferList * const _Nonnull bufferList, size_type frameCount) noexcept;

	/// Reads audio and advances the read position.
	///
	/// If fewer than the requested number of frames are available the remainder of the audio buffer list will be set to zero.
	/// @note This method is only safe to call from the consumer.
	/// @param bufferList An audio buffer list to receive the data.
	/// @param frameCount The desired number of audio frames to read.
	/// @return The number of audio frames actually read.
	size_type Read(AudioBufferList * const _Nonnull bufferList, size_type frameCount) noexcept;

	// MARK: Discarding Audio

	/// Skips audio and advances the read position.
	/// @note This method is only safe to call from the consumer.
	/// @param frameCount The desired number of audio frames to skip.
	/// @return The number of audio frames actually skipped.
	size_type Skip(size_type frameCount) noexcept;

	/// Drains the ring buffer.
	/// @note This method is only safe to call from the consumer.
	void Drain() noexcept;

	/// Resets the ring buffer.
	/// @note This method is only safe to call from the producer.
	void Reset() noexcept;

private:
	/// The memory buffers holding the data, consisting of channel pointers and buffers allocated in one chunk.
	void * _Nonnull * _Nullable buffers_{nullptr};

	/// The per-channel capacity of ``buffers_`` in audio frames.
	size_type capacity_{0};
	/// The per-channel capacity of ``buffers_`` in audio frames minus one.
	size_type capacityMask_{0};

	/// The free-running write location.
	std::atomic<size_type> writePosition_{0};
	/// The free-running read location.
	std::atomic<size_type> readPosition_{0};

	/// The ring buffer epoch, incremented by ``Reset``.
	std::atomic<size_type> epoch_{0};
	/// The epoch during the latest call to ``Read``.
	std::atomic<size_type> readEpoch_{0};

	static_assert(std::atomic<size_type>::is_always_lock_free, "Lock-free std::atomic<size_type> required");

	/// The format of the audio this ring buffer contains.
	CAStreamDescription format_{};
};

} /* namespace CXXCoreAudio */
