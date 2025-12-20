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
/// This class is thread safe when used from one reader thread and one writer thread.
class AudioRingBuffer final {
public:
	/// Unsigned integer type.
	using size_type = std::size_t;

	/// The minimum supported ring buffer size in audio frames.
	static constexpr size_type min_buffer_size = size_type{2};
	/// The maximum supported ring buffer size in audio frames.
	static constexpr size_type max_buffer_size = size_type{1} << (std::numeric_limits<size_type>::digits - 1);

	// MARK: Creation and Destruction

	/// Creates an empty ring buffer.
	/// @note ``Allocate`` must be called before the object may be used.
	AudioRingBuffer() noexcept = default;

	/// Creates a ring buffer with the specified buffer size.
	///
	/// The format-specific maximum size is the largest integral power of two not greater than std::numeric_limits<UInt32>::max() / format.mBytesPerFrame.
	///
	/// The limiting ring buffer size is the lesser of the maximum supported size and the format-specific maximum size.
	/// @note Only non-interleaved formats are supported.
	/// @note The ring buffer capacity will rounded to the smallest integral power of two that is not less than the specified size.
	/// @param format The format of the audio that will be written to and read from this buffer.
	/// @param size The desired buffer capacity per channel, in audio frames.
	/// @throw std::bad_alloc if memory could not be allocated or std::invalid_argument if the buffer size is not supported.
	AudioRingBuffer(const AudioStreamBasicDescription& format, size_type size);

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

	/// Allocates space for audio data.
	///
	/// The format-specific maximum size is the largest integral power of two not greater than std::numeric_limits<UInt32>::max() / format.mBytesPerFrame.
	///
	/// The limiting ring buffer size is the lesser of the maximum supported size and the format-specific maximum size.
	/// @note Only non-interleaved formats are supported.
	/// @note This method is not thread safe.
	/// @note The ring buffer capacity will rounded to the smallest integral power of two that is not less than the specified size.
	/// @param format The format of the audio that will be written to and read from this buffer.
	/// @param size The desired buffer capacity per channel, in audio frames.
	/// @return true on success, false if memory could not be allocated, the audio format is not supported, or the buffer size is not supported.
	bool Allocate(const AudioStreamBasicDescription& format, size_type size) noexcept;

	/// Frees any space allocated for data.
	/// @note This method is not thread safe.
	void Deallocate() noexcept;

	/// Resets the read and write positions to their default state, emptying the buffer.
	/// @note This method is not thread safe.
	void Reset() noexcept;

	// MARK: Buffer Information

	/// Returns the capacity of the ring buffer.
	/// @return The ring buffer capacity in audio frames.
	[[nodiscard]] size_type Capacity() const noexcept;

	/// Returns the amount of free space in the buffer.
	/// @return The number of audio frames of free space available for writing.
	[[nodiscard]] size_type FreeSpace() const noexcept;

	/// Returns the amount of audio in the buffer.
	/// @return The number of audio frames available for reading.
	[[nodiscard]] size_type AvailableFrames() const noexcept;

	/// Returns true if the ring buffer is empty.
	[[nodiscard]] bool IsEmpty() const noexcept;

	/// Returns true if the ring buffer is full.
	[[nodiscard]] bool IsFull() const noexcept;

	/// Returns the format of the audio in this ring buffer.
	[[nodiscard]] const CAStreamDescription& Format() const noexcept
	{
		return format_;
	}

	// MARK: Writing and Reading Audio

	/// Writes audio and advances the write position.
	/// @param bufferList An audio buffer list containing the data to copy.
	/// @param frameCount The desired number of audio frames to write.
	/// @param allowPartial Whether any audio frames should be written if the free space available for writing is less than frameCount.
	/// @return The number of audio frames actually written.
	size_type Write(const AudioBufferList * const _Nonnull bufferList, size_type frameCount, bool allowPartial = true) noexcept;

	/// Reads audio and advances the read position.
	/// @param bufferList An audio buffer list to receive the data.
	/// @param frameCount The desired number of audio frames to read.
	/// @param allowPartial Whether any audio frames should be read if the number of frames available for reading is less than frameCount.
	/// @return The number of audio frames actually read.
	size_type Read(AudioBufferList * const _Nonnull bufferList, size_type frameCount, bool allowPartial = true) noexcept;

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

	static_assert(std::atomic<size_type>::is_always_lock_free, "Lock-free std::atomic<size_type> required");

	/// The format of the audio this ring buffer contains.
	CAStreamDescription format_{};
};

} /* namespace CXXCoreAudio */
