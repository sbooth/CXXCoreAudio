//
// SPDX-FileCopyrightText: 2013 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXCoreAudio
//

#pragma once

#include <CXXCoreAudio/CAStreamDescription.hpp>
#include <CXXCoreAudio/malloc_ptr.hpp>

#include <CoreAudioTypes/CoreAudioTypes.h>

#include <algorithm>

namespace CXXCoreAudio {

/// Allocates and returns a variable-length AudioBufferList structure in a single allocation.
/// @note The allocation is performed using std::malloc.
/// @param format The format of the audio the buffer list will contain.
/// @param frameCapacity The desired buffer capacity in audio frames.
/// @return An AudioBufferList struct or null if an error occurred or memory could not be allocated.
[[nodiscard]] malloc_ptr<AudioBufferList> allocateAudioBufferList(const AudioStreamBasicDescription& format,
                                                                  UInt32 frameCapacity) noexcept;

/// A class managing an AudioBufferList structure along with a specific format, frame capacity, and frame length.
class CAAudioBuffer final {
  public:
    // MARK: Construction and Destruction

    /// Creates an empty buffer list.
    /// @note ``allocate`` must be called before the object may be used.
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
    bool allocate(const AudioStreamBasicDescription& format, UInt32 frameCapacity) noexcept;

    /// Deallocates the memory associated with this buffer list and sets the frame length and frame capacity to zero.
    void deallocate() noexcept;

    /// Clears the buffer list, setting the frame length to zero.
    /// @return true on success, false otherwise.
    bool clear() noexcept;

    /// Returns the length in audio frames of the data in this buffer list.
    [[nodiscard]] UInt32 frameLength() const noexcept;

    /// Set the length in audio frames of the data in this buffer list.
    /// @param frameLength The number of valid audio frames.
    /// @return true on success, false otherwise.
    bool setFrameLength(UInt32 frameLength) noexcept;

    /// Returns true if the frame length is zero.
    [[nodiscard]] bool isEmpty() const noexcept;

    /// Returns true if the frame length is equal to the frame capacity.
    [[nodiscard]] bool isFull() const noexcept;

    /// Returns the audio frame capacity.
    [[nodiscard]] UInt32 frameCapacity() const noexcept;

    // MARK: Format

    /// Returns the audio format of the buffer list.
    [[nodiscard]] const CAStreamDescription& format() const noexcept;

    // MARK: External Reading

    /// Sets the frame length to the frame capacity.
    ///
    /// This is normally called to prepare the buffer list for a read operation.
    bool prepareForReading() noexcept;

    /// Infers and updates the frame length using the mDataByteSize field of the managed AudioBufferList struct.
    ///
    /// This is normally called after data has been copied to the buffer list during a read operation.
    /// @return true on success, false otherwise.
    /// @throw std::logic_error
    bool inferFrameLength();

    // MARK: Buffer Utilities

    /// Prepends the contents of a buffer list.
    /// @note The format of buffer must match the format of this buffer list.
    /// @param buffer A buffer of audio data.
    /// @return The number of frames prepended.
    UInt32 prepend(const CAAudioBuffer& buffer) noexcept;

    /// Prepends a portion of the contents of a buffer list.
    /// @note The format of buffer must match the format of this buffer list.
    /// @param buffer A buffer of audio data.
    /// @param readOffset The location in buffer to start reading, in audio frames.
    /// @return The number of frames prepended.
    UInt32 prepend(const CAAudioBuffer& buffer, UInt32 readOffset) noexcept;

    /// Prepends a portion of the contents of a buffer list.
    /// @note The format of buffer must match the format of this buffer list.
    /// @param buffer A buffer of audio data.
    /// @param readOffset The location in buffer to start reading, in audio frames.
    /// @param frameLength The number of frames to prepend.
    /// @return The number of frames prepended
    UInt32 prepend(const CAAudioBuffer& buffer, UInt32 readOffset, UInt32 frameLength) noexcept;

    /// Appends the contents of a buffer list.
    /// @note The format of buffer must match the format of this buffer list.
    /// @param buffer A buffer of audio data.
    /// @return The number of frames appended.
    UInt32 append(const CAAudioBuffer& buffer) noexcept;

    /// Appends a portion of the contents of a buffer list.
    /// @note The format of buffer must match the format of this buffer list.
    /// @param buffer A buffer of audio data.
    /// @param readOffset The location in buffer to start reading, in audio frames.
    /// @return The number of frames appended
    UInt32 append(const CAAudioBuffer& buffer, UInt32 readOffset) noexcept;

    /// Appends a portion of the contents of a buffer list.
    /// @note The format of buffer must match the format of this buffer list.
    /// @param buffer A buffer of audio data.
    /// @param readOffset The location in buffer to start reading, in audio frames.
    /// @param frameLength The number of frames to append.
    /// @return The number of frames appended
    UInt32 append(const CAAudioBuffer& buffer, UInt32 readOffset, UInt32 frameLength) noexcept;

    /// Inserts the contents of a buffer list.
    /// @note The format of buffer must match the format of this buffer list.
    /// @param buffer A buffer of audio data.
    /// @param writeOffset The location in this buffer list to start writing, in audio frames.
    /// @return The number of frames inserted.
    UInt32 insert(const CAAudioBuffer& buffer, UInt32 writeOffset) noexcept;

    /// Inserts a portion of the contents of a buffer list.
    /// @note The format of buffer must match the format of this buffer list.
    /// @param buffer A buffer of audio data.
    /// @param readOffset The location in buffer to start reading, in audio frames.
    /// @param frameLength The number of frames to insert.
    /// @param writeOffset The location in this buffer list to start writing, in audio frames.
    /// @return The number of frames inserted.
    UInt32 insert(const CAAudioBuffer& buffer, UInt32 readOffset, UInt32 frameLength, UInt32 writeOffset) noexcept;

    /// Deletes frames from the beginning of this buffer list.
    /// @param frameLength The number of frames to delete.
    /// @return The number of frames deleted.
    UInt32 trimFirst(UInt32 frameLength) noexcept;

    /// Deletes frames from the end of this buffer list.
    /// @param frameLength The number of frames to delete.
    /// @return The number of frames deleted.
    UInt32 trimLast(UInt32 frameLength) noexcept;

    /// Deletes frames from this buffer list.
    /// @param offset The location to start deleting, in audio frames.
    /// @param frameLength The number of frames to delete.
    /// @return The number of frames deleted.
    UInt32 trim(UInt32 offset, UInt32 frameLength) noexcept;

    /// Fills the remainder of this buffer list with silence.
    /// @return The number of frames of silence appended.
    UInt32 fillRemainderWithSilence() noexcept;

    /// Appends silence to this buffer list.
    /// @param frameLength The number of frames to append.
    /// @return The number of frames of silence appended.
    UInt32 appendSilence(UInt32 frameLength) noexcept;

    /// Inserts silence in this buffer list.
    /// @param offset The location to start inserting, in audio frames.
    /// @param frameLength The number of frames to insert.
    /// @return The number of frames of silence inserted.
    UInt32 insertSilence(UInt32 offset, UInt32 frameLength) noexcept;

    // MARK: AudioBufferList Access

    /// Returns true if the managed AudioBufferList struct is not null.
    [[nodiscard]] explicit operator bool() const noexcept;

    /// Returns a pointer to the managed AudioBufferList struct.
    [[nodiscard]] AudioBufferList *_Nullable operator->() noexcept;

    /// Returns a pointer to the managed AudioBufferList struct.
    [[nodiscard]] operator AudioBufferList *const _Nullable() noexcept;

    /// Returns a const pointer to the managed AudioBufferList struct.
    [[nodiscard]] const AudioBufferList *_Nullable operator->() const noexcept;

    /// Returns a const pointer to the managed AudioBufferList struct.
    [[nodiscard]] operator const AudioBufferList *const _Nullable() const noexcept;

    // MARK: AudioBufferList Management

    /// Adopts an existing AudioBufferList struct.
    /// @note The object assumes responsibility for deallocating the passed AudioBufferList struct using std::free.
    /// @param bufferList The AudioBufferList struct to adopt.
    /// @param format The format of bufferList.
    /// @param frameCapacity The frame capacity of bufferList.
    /// @param frameLength The number of valid audio frames in bufferList.
    /// @return true on success, false otherwise.
    bool adopt(AudioBufferList *_Nonnull bufferList, const AudioStreamBasicDescription& format, UInt32 frameCapacity,
               UInt32 frameLength) noexcept;

    /// Releases ownership of managed AudioBufferList struct and returns it.
    /// @note The caller assumes responsibility for deallocating the returned AudioBufferList using std::free.
    [[nodiscard]] AudioBufferList *_Nullable release() noexcept;

  private:
    /// The managed AudioBufferList struct.
    AudioBufferList *_Nullable bufferList_{nullptr};
    /// The format of ``bufferList_``.
    CAStreamDescription format_{};
    /// The capacity of ``bufferList_`` in frames.
    UInt32 frameCapacity_{0};
    /// The number of valid frames in ``bufferList_``.
    UInt32 frameLength_{0};
};

// MARK: - Implementation -

// MARK: Buffer Management

inline bool CAAudioBuffer::clear() noexcept {
    return setFrameLength(0);
}

inline UInt32 CAAudioBuffer::frameLength() const noexcept {
    return frameLength_;
}

inline bool CAAudioBuffer::isEmpty() const noexcept {
    return frameLength_ == 0;
}

inline bool CAAudioBuffer::isFull() const noexcept {
    return frameLength_ == frameCapacity_;
}

inline UInt32 CAAudioBuffer::frameCapacity() const noexcept {
    return frameCapacity_;
}

// MARK: Format

inline const CAStreamDescription& CAAudioBuffer::format() const noexcept {
    return format_;
}

// MARK: External Reading

inline bool CAAudioBuffer::prepareForReading() noexcept {
    return setFrameLength(frameCapacity_);
}

// MARK: Buffer Utilities

inline UInt32 CAAudioBuffer::prepend(const CAAudioBuffer& buffer) noexcept {
    return insert(buffer, 0, buffer.frameLength_, 0);
}

inline UInt32 CAAudioBuffer::prepend(const CAAudioBuffer& buffer, UInt32 readOffset) noexcept {
    if (readOffset > buffer.frameLength_) {
        return 0;
    }
    return insert(buffer, readOffset, (buffer.frameLength_ - readOffset), 0);
}

inline UInt32 CAAudioBuffer::prepend(const CAAudioBuffer& buffer, UInt32 readOffset, UInt32 frameLength) noexcept {
    return insert(buffer, readOffset, frameLength, 0);
}

inline UInt32 CAAudioBuffer::append(const CAAudioBuffer& buffer) noexcept {
    return insert(buffer, 0, buffer.frameLength_, frameLength_);
}

inline UInt32 CAAudioBuffer::append(const CAAudioBuffer& buffer, UInt32 readOffset) noexcept {
    if (readOffset > buffer.frameLength_) {
        return 0;
    }
    return insert(buffer, readOffset, (buffer.frameLength_ - readOffset), frameLength_);
}

inline UInt32 CAAudioBuffer::append(const CAAudioBuffer& buffer, UInt32 readOffset, UInt32 frameLength) noexcept {
    return insert(buffer, readOffset, frameLength, frameLength_);
}

inline UInt32 CAAudioBuffer::insert(const CAAudioBuffer& buffer, UInt32 writeOffset) noexcept {
    return insert(buffer, 0, buffer.frameLength_, writeOffset);
}

inline UInt32 CAAudioBuffer::trimFirst(UInt32 frameLength) noexcept {
    return trim(0, frameLength);
}

inline UInt32 CAAudioBuffer::trimLast(UInt32 frameLength) noexcept {
    const UInt32 framesToTrim = std::min(frameLength, frameLength_);
    setFrameLength(frameLength_ - framesToTrim);
    return framesToTrim;
}

inline UInt32 CAAudioBuffer::fillRemainderWithSilence() noexcept {
    return insertSilence(frameLength_, frameCapacity_ - frameLength_);
}

inline UInt32 CAAudioBuffer::appendSilence(UInt32 frameLength) noexcept {
    return insertSilence(frameLength_, frameLength);
}

// MARK: AudioBufferList Access

inline CAAudioBuffer::operator bool() const noexcept {
    return bufferList_ != nullptr;
}

inline AudioBufferList *_Nullable CAAudioBuffer::operator->() noexcept {
    return bufferList_;
}

inline CAAudioBuffer::operator AudioBufferList *const _Nullable() noexcept {
    return bufferList_;
}

inline const AudioBufferList *_Nullable CAAudioBuffer::operator->() const noexcept {
    return bufferList_;
}

inline CAAudioBuffer::operator const AudioBufferList *const _Nullable() const noexcept {
    return bufferList_;
}

} /* namespace CXXCoreAudio */
