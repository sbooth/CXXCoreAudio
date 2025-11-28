//
// Copyright © 2013-2025 Stephen F. Booth
// Part of https://github.com/sbooth/CXXCoreAudio
// MIT license
//

#pragma once

#import <algorithm>

#import <CoreAudioTypes/CoreAudioTypes.h>

#import <CXXCoreAudio/CAStreamDescription.hpp>

namespace CXXCoreAudio {

/// Allocates and returns a variable-length AudioBufferList structure in a single allocation.
/// @note The allocation is performed using std::malloc and should be deallocated using std::free.
/// @param format The format of the audio the buffer list will contain.
/// @param frameCapacity The desired buffer capacity in audio frames.
/// @return An AudioBufferList struct or null if an error occurred or memory could not be allocated.
AudioBufferList * _Nullable AllocateAudioBufferList(const AudioStreamBasicDescription& format, UInt32 frameCapacity) noexcept;

/// A class containing an AudioBufferList with a specific format, frame capacity, and frame length.
class CAAudioBuffer final {
public:
	// MARK: Creation and Destruction

	/// Creates an empty buffer list.
	/// @note ``Allocate`` must be called before the object may be used.
	CAAudioBuffer() noexcept = default;

	// This class is non-copyable
	CAAudioBuffer(const CAAudioBuffer&) = delete;

	/// Creates a buffer list by moving the contents of another.
	CAAudioBuffer(CAAudioBuffer&& other) noexcept;

	// This class is non-assignable
	CAAudioBuffer& operator=(const CAAudioBuffer&) = delete;

	/// Replaces the buffer list with the moved contents of another.
	CAAudioBuffer& operator=(CAAudioBuffer&& other) noexcept;

	/// Destroys the buffer list and releases all associated resources.
	~CAAudioBuffer() noexcept;

	/// Creates a buffer list.
	/// @param format The format of the audio the buffer list will contain.
	/// @param frameCapacity The desired buffer capacity in audio frames.
	/// @throw std::invalid_argument, std::bad_alloc
	CAAudioBuffer(const AudioStreamBasicDescription& format, UInt32 frameCapacity);

	// MARK: Buffer Management

	/// Allocates space for audio.
	/// @param format The format of the audio the buffer list will contain.
	/// @param frameCapacity The desired buffer capacity in audio frames.
	/// @return true on success, false if an error occurred or memory could not be allocated.
	bool Allocate(const AudioStreamBasicDescription& format, UInt32 frameCapacity) noexcept;

	/// Deallocates the memory associated with this buffer list and sets the frame length and frame capacity to zero.
	void Deallocate() noexcept;

	/// Clears the buffer list, setting the frame length to zero.
	/// @return true on success, false otherwise.
	bool Clear() noexcept
	{
		return SetFrameLength(0);
	}

	/// Returns the length in audio frames of the data in this buffer list.
	UInt32 FrameLength() const noexcept
	{
		return frameLength_;
	}

	/// Set the length in audio frames of the data in this buffer list.
	/// @param frameLength The number of valid audio frames.
	/// @return true on success, false otherwise.
	bool SetFrameLength(UInt32 frameLength) noexcept;

	/// Returns true if the frame length is zero.
	bool IsEmpty() const noexcept
	{
		return frameLength_ == 0;
	}

	/// Returns true if the frame length is equal to the frame capacity.
	bool IsFull() const noexcept
	{
		return frameLength_ == frameCapacity_;
	}

	/// Returns the audio frame capacity.
	UInt32 FrameCapacity() const noexcept
	{
		return frameCapacity_;
	}

	// MARK: Format

	/// Returns the audio format of the buffer list.
	const CAStreamDescription& Format() const noexcept
	{
		return format_;
	}

	// MARK: External Reading

	/// Sets the frame length to the frame capacity.
	///
	/// This is normally called to prepare the buffer list for a read operation.
	bool PrepareForReading() noexcept
	{
		return SetFrameLength(frameCapacity_);
	}

	/// Infers and updates the frame length using the mDataByteSize field of the internal AudioBufferList.
	///
	/// This is normally called after data has been copied to the buffer list during a read operation.
	/// @return true on success, false otherwise.
	/// @throw std::logic_error
	bool InferFrameLength();

	// MARK: Buffer Utilities

	/// Prepends the contents of a buffer list.
	/// @note The format of buffer must match the format of this buffer list.
	/// @param buffer A buffer of audio data.
	/// @return The number of frames prepended.
	UInt32 Prepend(const CAAudioBuffer& buffer) noexcept
	{
		return Insert(buffer, 0, buffer.frameLength_, 0);
	}

	/// Prepends a portion of the contents of a buffer list.
	/// @note The format of buffer must match the format of this buffer list.
	/// @param buffer A buffer of audio data.
	/// @param readOffset The location in buffer to start reading, in audio frames.
	/// @return The number of frames prepended.
	UInt32 Prepend(const CAAudioBuffer& buffer, UInt32 readOffset) noexcept
	{
		if(readOffset > buffer.frameLength_)
			return 0;
		return Insert(buffer, readOffset, (buffer.frameLength_ - readOffset), 0);
	}

	/// Prepends a portion of the contents of a buffer list.
	/// @note The format of buffer must match the format of this buffer list.
	/// @param buffer A buffer of audio data.
	/// @param readOffset The location in buffer to start reading, in audio frames.
	/// @param frameLength The number of frames to prepend.
	/// @return The number of frames prepended
	UInt32 Prepend(const CAAudioBuffer& buffer, UInt32 readOffset, UInt32 frameLength) noexcept
	{
		return Insert(buffer, readOffset, frameLength, 0);
	}

	/// Appends the contents of a buffer list.
	/// @note The format of buffer must match the format of this buffer list.
	/// @param buffer A buffer of audio data.
	/// @return The number of frames appended.
	UInt32 Append(const CAAudioBuffer& buffer) noexcept
	{
		return Insert(buffer, 0, buffer.frameLength_, frameLength_);
	}

	/// Appends a portion of the contents of a buffer list.
	/// @note The format of buffer must match the format of this buffer list.
	/// @param buffer A buffer of audio data.
	/// @param readOffset The location in buffer to start reading, in audio frames.
	/// @return The number of frames appended
	UInt32 Append(const CAAudioBuffer& buffer, UInt32 readOffset) noexcept
	{
		if(readOffset > buffer.frameLength_)
			return 0;
		return Insert(buffer, readOffset, (buffer.frameLength_ - readOffset), frameLength_);
	}

	/// Appends a portion of the contents of a buffer list.
	/// @note The format of buffer must match the format of this buffer list.
	/// @param buffer A buffer of audio data.
	/// @param readOffset The location in buffer to start reading, in audio frames.
	/// @param frameLength The number of frames to append.
	/// @return The number of frames appended
	UInt32 Append(const CAAudioBuffer& buffer, UInt32 readOffset, UInt32 frameLength) noexcept
	{
		return Insert(buffer, readOffset, frameLength, frameLength_);
	}

	/// Inserts the contents of a buffer list.
	/// @note The format of buffer must match the format of this buffer list.
	/// @param buffer A buffer of audio data.
	/// @param writeOffset The location in this buffer list to start writing, in audio frames.
	/// @return The number of frames inserted.
	UInt32 Insert(const CAAudioBuffer& buffer, UInt32 writeOffset) noexcept
	{
		return Insert(buffer, 0, buffer.frameLength_, writeOffset);
	}

	/// Inserts a portion of the contents of a buffer list.
	/// @note The format of buffer must match the format of this buffer list.
	/// @param buffer A buffer of audio data.
	/// @param readOffset The location in buffer to start reading, in audio frames.
	/// @param frameLength The number of frames to insert.
	/// @param writeOffset The location in this buffer list to start writing, in audio frames.
	/// @return The number of frames inserted.
	UInt32 Insert(const CAAudioBuffer& buffer, UInt32 readOffset, UInt32 frameLength, UInt32 writeOffset) noexcept;

	/// Deletes frames from the beginning of this buffer list.
	/// @param frameLength The number of frames to delete.
	/// @return The number of frames deleted.
	UInt32 TrimFirst(UInt32 frameLength) noexcept
	{
		return Trim(0, frameLength);
	}

	/// Deletes frames from the end of this buffer list.
	/// @param frameLength The number of frames to delete.
	/// @return The number of frames deleted.
	UInt32 TrimLast(UInt32 frameLength) noexcept
	{
		const UInt32 framesToTrim = std::min(frameLength, frameLength_);
		SetFrameLength(frameLength_ - framesToTrim);
		return framesToTrim;
	}

	/// Deletes frames from this buffer list.
	/// @param offset The location to start deleting, in audio frames.
	/// @param frameLength The number of frames to delete.
	/// @return The number of frames deleted.
	UInt32 Trim(UInt32 offset, UInt32 frameLength) noexcept;

	/// Fills the remainder of this buffer list with silence.
	/// @return The number of frames of silence appended.
	UInt32 FillRemainderWithSilence() noexcept
	{
		return InsertSilence(frameLength_, frameCapacity_ - frameLength_);
	}

	/// Appends silence to this buffer list.
	/// @param frameLength The number of frames to append.
	/// @return The number of frames of silence appended.
	UInt32 AppendSilence(UInt32 frameLength) noexcept
	{
		return InsertSilence(frameLength_, frameLength);
	}

	/// Inserts silence in this buffer list.
	/// @param offset The location to start inserting, in audio frames.
	/// @param frameLength The number of frames to insert.
	/// @return The number of frames of silence inserted.
	UInt32 InsertSilence(UInt32 offset, UInt32 frameLength) noexcept;

	// MARK: AudioBufferList Access

	/// Returns true if this object's internal AudioBufferList is not null.
	explicit operator bool() const noexcept 							{ return bufferList_ != nullptr; }

	/// Returns a pointer to this object's internal AudioBufferList.
	AudioBufferList * _Nullable GetBufferList() noexcept 				{ return bufferList_; }
	/// Returns a const pointer to this object's internal AudioBufferList.
	const AudioBufferList * _Nullable GetBufferList() const noexcept 	{ return bufferList_; }

	/// Returns a pointer to this object's internal AudioBufferList.
	AudioBufferList * _Nullable operator->() noexcept 					{ return bufferList_; }
	/// Returns a const pointer to this object's internal AudioBufferList.
	const AudioBufferList * _Nullable operator->() const noexcept 		{ return bufferList_; }

	/// Returns a pointer to this object's internal AudioBufferList.
	operator AudioBufferList * const _Nullable () noexcept 				{ return bufferList_; }
	/// Returns a const pointer to this object's internal AudioBufferList.
	operator const AudioBufferList * const _Nullable () const noexcept 	{ return bufferList_; }

	// MARK: AudioBufferList Management

	/// Adopts an existing AudioBufferList.
	/// @note The object assumes responsibility for deallocating the passed AudioBufferList using std::free.
	/// @param bufferList The AudioBufferList to adopt.
	/// @param format The format of bufferList.
	/// @param frameCapacity The frame capacity of bufferList.
	/// @param frameLength The number of valid audio frames in bufferList.
	/// @return true on success, false otherwise.
	bool AdoptABL(AudioBufferList * _Nonnull bufferList, const AudioStreamBasicDescription& format, UInt32 frameCapacity, UInt32 frameLength) noexcept;

	/// Releases ownership of the object's internal AudioBufferList and returns it.
	/// @note The caller assumes responsibility for deallocating the returned AudioBufferList using std::free.
	AudioBufferList * _Nullable Release() noexcept;

private:
	/// The underlying AudioBufferList struct.
	AudioBufferList * _Nullable bufferList_{nullptr};
	/// The format of ``bufferList_``.
	CAStreamDescription format_{};
	/// The capacity of ``bufferList_`` in frames.
	UInt32 frameCapacity_{0};
	/// The number of valid frames in ``bufferList_``.
	UInt32 frameLength_{0};
};

} /* namespace CXXCoreAudio */
