//
// Copyright © 2013-2025 Stephen F. Booth
// Part of https://github.com/sbooth/CXXCoreAudio
// MIT license
//

#pragma once

#import <atomic>

#import <CoreAudioTypes/CoreAudioTypes.h>

#import <CXXCoreAudio/CAStreamDescription.hpp>

namespace CXXCoreAudio {

/// A timestamped SPSC ring buffer supporting non-interleaved audio based on Apple's CARingBuffer.
///
/// This class is thread safe when used from one reader thread and one writer thread.
class CARingBuffer final {
public:

	// MARK: Creation and Destruction

	/// Creates an empty ring buffer.
	/// @note ``Allocate`` must be called before the object may be used.
	CARingBuffer() noexcept = default;

	/// Creates a ring buffer with the specified buffer size.
	///
	/// The minimum buffer capacity is two audio frames and the maximum capacity is 0x80000000 audio frames.
	///
	/// The format-specific capacity is the largest integral power of two not greater than std::numeric_limits<uint32_t>::max() / format.mBytesPerFrame.
	///
	/// The limiting buffer capacity is the lesser of the maximum capacity and the format-specific capacity.
	/// @note Only non-interleaved formats are supported.
	/// @note The usable ring buffer capacity will be one less than the smallest integral power of two that is not less than the specified size.
	/// @param format The format of the audio that will be written to and read from this buffer.
	/// @param size The desired buffer capacity per channel, in audio frames.
	/// @throw std::bad_alloc if memory could not be allocated or std::invalid_argument if the buffer size is not supported.
	CARingBuffer(const AudioStreamBasicDescription& format, uint32_t size);

	// This class is non-copyable
	CARingBuffer(const CARingBuffer&) = delete;

	/// Creates a ring buffer by moving the contents of another ring buffer.
	/// @note This method is not thread safe for the ring buffer being moved.
	/// @param other The ring buffer to move.
	CARingBuffer(CARingBuffer&& other) noexcept;

	// This class is non-assignable
	CARingBuffer& operator=(const CARingBuffer&) = delete;

	/// Moves the contents of another ring buffer into this ring buffer.
	/// @note This method is not thread safe.
	/// @param other The ring buffer to move.
	CARingBuffer& operator=(CARingBuffer&& other) noexcept;

	/// Destroys the ring buffer and releases all associated resources.
	~CARingBuffer() noexcept;

	// MARK: Buffer Management

	/// Allocates space for audio data.
	///
	/// The minimum buffer capacity is two audio frames and the maximum capacity is 0x80000000 audio frames.
	///
	/// The format-specific capacity is the largest integral power of two not greater than std::numeric_limits<uint32_t>::max() / format.mBytesPerFrame.
	///
	/// The limiting buffer capacity is the lesser of the maximum capacity and the format-specific capacity.
	/// @note Only non-interleaved formats are supported.
	/// @note This method is not thread safe.
	/// @note The usable ring buffer capacity will be one less than the smallest integral power of two that is not less than the specified size.
	/// @param format The format of the audio that will be written to and read from this buffer.
	/// @param size The desired buffer capacity per channel, in audio frames.
	/// @return true on success, false if memory could not be allocated, the audio format is not supported, or the buffer size is not supported.
	bool Allocate(const AudioStreamBasicDescription& format, uint32_t size) noexcept;

	/// Frees any space allocated for data.
	/// @note This method is not thread safe.
	void Deallocate() noexcept;

	/// Resets the buffer start and end times to zero, emptying the buffer.
	/// @note This method is not thread safe.
	void Reset() noexcept;

	// MARK: Buffer Information

	/// Returns the usable capacity of the ring buffer in audio frames.
	/// @return The usable ring buffer capacity in audio frames.
	uint32_t Capacity() const noexcept;

	/// Gets the time bounds of the audio contained in the ring buffer.
	/// @param startTime The starting sample time of audio contained in the buffer.
	/// @param endTime The end sample time of audio contained in the buffer.
	/// @return true on success, false on error.
	bool GetTimeBounds(int64_t& startTime, int64_t& endTime) const noexcept;

	/// Returns the format of the audio in this ring buffer.
	const CAStreamDescription& Format() const noexcept
	{
		return format_;
	}

	// MARK: Reading and Writing Audio

	/// Reads audio from the ring buffer.
	///
	/// The sample times should normally increase sequentially, although gaps are filled with silence.
	/// A sufficiently large gap effectively empties the buffer before storing the new data.
	/// @note Negative time stamps are not supported.
	/// @note If the time stamp is less than the previous sample time the behavior is undefined.
	/// @param destination An audio buffer list to receive the data.
	/// @param count The desired number of audio frames to read.
	/// @param time The sample time of the first frame to read.
	/// @return true on success, false on error.
	bool Read(AudioBufferList * const _Nonnull destination, uint32_t count, int64_t time) noexcept;

	/// Writes audio to the ring buffer.
	/// @note Negative time stamps are not supported.
	/// @param source An audio buffer list containing the data to copy.
	/// @param count The desired number of audio frames to write.
	/// @param time The sample time of the first frame to write.
	/// @return true on success, false on error.
	bool Write(const AudioBufferList * const _Nonnull source, uint32_t count, int64_t time) noexcept;

private:
	/// Returns the byte offset of a frame number.
	uint32_t FrameByteOffset(int64_t frameNumber) const noexcept
	{
		return (static_cast<uint64_t>(frameNumber) & capacityMask_) * format_.mBytesPerFrame;
	}

	/// The memory buffers holding the data, consisting of channel pointers and buffers allocated in one chunk.
	void * _Nonnull * _Nullable buffers_{nullptr};

	/// The per-channel capacity of ``buffers_`` in audio frames.
	uint32_t capacity_{0};
	/// The per-channel capacity of ``buffers_`` in audio frames minus one.
	uint32_t capacityMask_{0};

	/// A range of valid sample times in the buffer.
	struct TimeBounds final {
		/// The starting sample time.
		int64_t startTime_{0};
		/// The ending sample time.
		int64_t endTime_{0};
		/// The value of timeBoundsQueueCounter_ when the struct was modified.
		std::atomic_uint64_t updateCounter_{0};
		static_assert(std::atomic_uint64_t::is_always_lock_free, "Lock-free std::atomic_uint64_t required");
	};

	/// The number of elements in the time bounds queue.
	static const uint32_t sTimeBoundsQueueSize{32};
	/// Mask value used to wrap time bounds counters.
	static const uint32_t sTimeBoundsQueueMask{sTimeBoundsQueueSize - 1};

	/// Array of TimeBounds structures.
	TimeBounds timeBoundsQueue_[sTimeBoundsQueueSize];

	/// Monotonically increasing counter incremented when the buffer's time bounds changes.
	std::atomic_uint64_t timeBoundsQueueCounter_{0};
	static_assert(std::atomic_uint64_t::is_always_lock_free, "Lock-free std::atomic_uint64_t required");

	/// The format of the audio this buffer contains.
	CAStreamDescription format_{};
};

} /* namespace CXXCoreAudio */
