//
// Copyright © 2013-2025 Stephen F. Booth
// Part of https://github.com/sbooth/CXXCoreAudio
// MIT license
//

#pragma once

#import <atomic>

#import <CoreAudioTypes/CoreAudioTypes.h>

#import "CAStreamDescription.hpp"

namespace CXXCoreAudio {

/// A lock-free SPSC audio ring buffer supporting non-interleaved audio.
///
/// This class is thread safe when used from one reader thread and one writer thread.
class AudioRingBuffer final {
public:

	// MARK: Creation and Destruction

	/// Creates an empty ring buffer.
	/// @note ``Allocate`` must be called before the object may be used.
	AudioRingBuffer() noexcept = default;

	/// Creates a ring buffer with the specified buffer size.
	/// @note Buffer sizes from 2 to 2,147,483,648 (0x80000000) bytes are supported.
	/// @note The usable ring buffer capacity will be one less than the smallest integral power of two that is not less than the specified size.
	/// @param size The desired buffer size, in audio frames.
	/// @throw std::bad_alloc if memory could not be allocated or std::invalid_argument if the buffer size is not supported.
	explicit AudioRingBuffer(const CAStreamDescription& format, uint32_t size);

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
	/// @note Only interleaved formats are supported.
	/// @note This method is not thread safe.
	/// @note Capacities from 2 to 2,147,483,648 (0x80000000) frames are supported
	/// @note The usable ring buffer capacity will be one less than the smallest integral power of two that is not less than the specified size.
	/// @param format The format of the audio that will be written to and read from this buffer.
	/// @param size The desired capacity, in audio frames.
	/// @return true on success, false if memory could not be allocated, the audio format is not supported, or the buffer size is not supported.
	bool Allocate(const CAStreamDescription& format, uint32_t size) noexcept;

	/// Frees any space allocated for data.
	/// @note This method is not thread safe.
	void Deallocate() noexcept;

	/// Resets the read and write positions to their default state, emptying the buffer.
	/// @note This method is not thread safe.
	void Reset() noexcept;

	// MARK: Buffer Information

	/// Returns the usable capacity of the ring buffer in audio frames.
	/// @return The usable ring buffer capacity in audio frames.
	uint32_t Capacity() const noexcept;

	/// Returns the number of frames of audio available for reading.
	/// @return The number of audio frames available to read.
	uint32_t AvailableReadCount() const noexcept;

	/// Returns the number of audio frames of free space available for writing.
	/// @return The number of audio frames available to write.
	uint32_t AvailableWriteCount() const noexcept;

	/// Returns the format of the audio in this ring buffer.
	const CAStreamDescription& Format() const noexcept
	{
		return format_;
	}

	// MARK: Reading and Writing Audio

	/// Reads audio and advances the read position.
	/// @param destination An audio buffer list to receive the data.
	/// @param count The desired number of audio frames to read.
	/// @param allowPartial Whether any audio frames should be read if the number of frames available for reading is less than count.
	/// @return The number of audio frames actually read.
	uint32_t Read(AudioBufferList * const _Nonnull destination, uint32_t count, bool allowPartial = true) noexcept;

	/// Writes audio and advances the write position.
	/// @param source An audio buffer list containing the data to copy.
	/// @param count The desired number of audio frames to write.
	/// @param allowPartial Whether any audio frames should be written if the free space available for writing is less than count.
	/// @return The number of audio frames actually written.
	uint32_t Write(const AudioBufferList * const _Nonnull source, uint32_t count, bool allowPartial = true) noexcept;

private:
	/// The memory buffers holding the data, consisting of channel pointers and buffers allocated in one chunk.
	void * _Nonnull * _Nullable buffers_{nullptr};

	/// The per-channel capacity of buffer_ in audio frames.
	uint32_t capacity_{0};
	/// The per-channel capacity of buffer_ in audio frames minus one.
	uint32_t capacityMask_{0};

	/// The offset into buffer_ of the write location.
	std::atomic_uint32_t writePosition_{0};
	/// The offset into buffer_ of the read location.
	std::atomic_uint32_t readPosition_{0};

	static_assert(std::atomic_uint32_t::is_always_lock_free, "Lock-free std::atomic_uint32_t required");

	/// The format of the audio this buffer contains.
	CAStreamDescription format_{};
};

} /* namespace CXXCoreAudio */
