//
// SPDX-FileCopyrightText: 2013 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXCoreAudio
//

#import "CAAudioBuffer.hpp"

#import <algorithm>
#import <cstdlib>
#import <limits>
#import <new>
#import <utility>

CXXCoreAudio::malloc_ptr<AudioBufferList>
CXXCoreAudio::AllocateAudioBufferList(const AudioStreamBasicDescription& format, UInt32 frameCapacity) noexcept {
    if (format.mBytesPerFrame == 0 || format.mChannelsPerFrame == 0 ||
        frameCapacity > (std::numeric_limits<UInt32>::max() / format.mBytesPerFrame))
        return nullptr;

    const auto bufferDataSize = frameCapacity * format.mBytesPerFrame;
    const auto bufferCount = (format.mFormatFlags & kAudioFormatFlagIsNonInterleaved) ? format.mChannelsPerFrame : 1;
    const auto bufferListSize = offsetof(AudioBufferList, mBuffers) + (sizeof(AudioBuffer) * bufferCount);
    const auto allocationSize = bufferListSize + (bufferDataSize * bufferCount);

    auto allocation = std::malloc(allocationSize);
    if (!allocation)
        return nullptr;

    // Zero the entire allocation
    std::memset(allocation, 0, allocationSize);

    // Assign the buffers
    auto address = reinterpret_cast<uintptr_t>(allocation);

    auto abl = static_cast<AudioBufferList *>(reinterpret_cast<void *>(address));
    abl->mNumberBuffers = bufferCount;

    for (UInt32 i = 0; i < bufferCount; ++i) {
        abl->mBuffers[i].mNumberChannels =
              (format.mFormatFlags & kAudioFormatFlagIsNonInterleaved) ? 1 : format.mChannelsPerFrame;
        abl->mBuffers[i].mDataByteSize = bufferDataSize;
        abl->mBuffers[i].mData = reinterpret_cast<void *>(address + bufferListSize + (bufferDataSize * i));
    }

    return malloc_ptr<AudioBufferList>{abl};
}

#if false
CXXCoreAudio::CAAudioBuffer::CAAudioBuffer(const CAAudioBuffer& other)
  : CAAudioBuffer{other.format_, other.frameCapacity_} {
    Insert(other, 0);
}
#endif

CXXCoreAudio::CAAudioBuffer::CAAudioBuffer(CAAudioBuffer&& other) noexcept
  : bufferList_{std::exchange(other.bufferList_, nullptr)},
    format_{std::exchange(other.format_, {})},
    frameCapacity_{std::exchange(other.frameCapacity_, 0)},
    frameLength_{std::exchange(other.frameLength_, 0)} {}

#if false
CXXCoreAudio::CAAudioBuffer& CXXCoreAudio::CAAudioBuffer::operator=(const CAAudioBuffer& other) {
    if(this != &other) {
        if(!Allocate(other.format_, other.frameCapacity_))
            throw std::bad_alloc();
        Insert(other, 0);
    }
    return *this;
}
#endif

CXXCoreAudio::CAAudioBuffer& CXXCoreAudio::CAAudioBuffer::operator=(CAAudioBuffer&& other) noexcept {
    if (this != &other) {
        std::free(bufferList_);
        bufferList_ = std::exchange(other.bufferList_, nullptr);
        format_ = std::exchange(other.format_, {});
        frameCapacity_ = std::exchange(other.frameCapacity_, 0);
        frameLength_ = std::exchange(other.frameLength_, 0);
    }
    return *this;
}

CXXCoreAudio::CAAudioBuffer::~CAAudioBuffer() noexcept {
    std::free(bufferList_);
}

CXXCoreAudio::CAAudioBuffer::CAAudioBuffer(const AudioStreamBasicDescription& format, UInt32 frameCapacity)
  : CAAudioBuffer{} {
    if (format.mBytesPerFrame == 0 || format.mChannelsPerFrame == 0)
        throw std::invalid_argument("invalid format");
    if (frameCapacity == 0 || frameCapacity > (std::numeric_limits<UInt32>::max() / format.mBytesPerFrame))
        throw std::invalid_argument("invalid frame capacity");
    if (!Allocate(format, frameCapacity))
        throw std::bad_alloc();
}

// MARK: Buffer Management

bool CXXCoreAudio::CAAudioBuffer::Allocate(const AudioStreamBasicDescription& format, UInt32 frameCapacity) noexcept {
    if (format.mBytesPerFrame == 0 || format.mChannelsPerFrame == 0 || frameCapacity == 0 ||
        frameCapacity > (std::numeric_limits<UInt32>::max() / format.mBytesPerFrame))
        return false;

    Deallocate();

    auto bufferList = AllocateAudioBufferList(format, frameCapacity);
    if (!bufferList)
        return false;
    bufferList_ = bufferList.release();

    format_ = format;
    frameCapacity_ = frameCapacity;
    frameLength_ = 0;

    return true;
}

void CXXCoreAudio::CAAudioBuffer::Deallocate() noexcept {
    if (bufferList_) {
        std::free(bufferList_);
        bufferList_ = nullptr;

        format_.Reset();

        frameCapacity_ = 0;
        frameLength_ = 0;
    }
}

bool CXXCoreAudio::CAAudioBuffer::SetFrameLength(UInt32 frameLength) noexcept {
    if (!bufferList_ || frameLength > frameCapacity_)
        return false;
    frameLength_ = frameLength;
    for (UInt32 i = 0; i < bufferList_->mNumberBuffers; ++i)
        bufferList_->mBuffers[i].mDataByteSize = frameLength_ * format_.mBytesPerFrame;
    return true;
}

bool CXXCoreAudio::CAAudioBuffer::InferFrameLength() {
    if (!bufferList_ || format_.mBytesPerFrame == 0)
        return false;

    // Verify frame length is within capacity
    const auto buffer0ByteSize = bufferList_->mBuffers[0].mDataByteSize;
    const auto frameLength = buffer0ByteSize / format_.mBytesPerFrame;
    if (frameLength > frameCapacity_)
        throw std::logic_error("bufferList_->mBuffers[0].mDataByteSize / format_.mBytesPerFrame > frameCapacity_");

    // Verify all buffers have same byte size
    for (UInt32 i = 0; i < bufferList_->mNumberBuffers; ++i) {
        if (bufferList_->mBuffers[i].mDataByteSize != buffer0ByteSize)
            throw std::logic_error("inconsistent values for mBuffers[].mDataByteSize");
    }

    frameLength_ = frameLength;
    return true;
}

// MARK: Buffer Utilities

UInt32 CXXCoreAudio::CAAudioBuffer::Insert(const CAAudioBuffer& buffer, UInt32 readOffset, UInt32 frameLength,
                                           UInt32 writeOffset) noexcept {
    if (format_ != buffer.format_)
        // throw std::invalid_argument("invalid audio format");
        return 0;

    if (readOffset > buffer.frameLength_ || writeOffset > frameLength_ || frameLength == 0 || buffer.frameLength_ == 0)
        return 0;

    const auto framesToInsert =
          std::min(frameCapacity_ - frameLength_, std::min(frameLength, buffer.frameLength_ - readOffset));
    const auto framesToMove = frameLength_ - writeOffset;
    if (framesToMove) {
        const auto moveToOffset = writeOffset + framesToInsert;
        for (UInt32 i = 0; i < bufferList_->mNumberBuffers; ++i) {
            const auto data = reinterpret_cast<uintptr_t>(bufferList_->mBuffers[i].mData);
            std::memmove(reinterpret_cast<void *>(data + (moveToOffset * format_.mBytesPerFrame)),
                         reinterpret_cast<const void *>(data + (writeOffset * format_.mBytesPerFrame)),
                         framesToMove * format_.mBytesPerFrame);
        }
    }

    if (framesToInsert) {
        for (UInt32 i = 0; i < buffer.bufferList_->mNumberBuffers; ++i)
            std::memcpy(
                  reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(bufferList_->mBuffers[i].mData) +
                                           (writeOffset * format_.mBytesPerFrame)),
                  reinterpret_cast<const void *>(reinterpret_cast<uintptr_t>(buffer.bufferList_->mBuffers[i].mData) +
                                                 (readOffset * format_.mBytesPerFrame)),
                  framesToInsert * format_.mBytesPerFrame);

        SetFrameLength(frameLength_ + framesToInsert);
    }

    return framesToInsert;
}

UInt32 CXXCoreAudio::CAAudioBuffer::Trim(UInt32 offset, UInt32 frameLength) noexcept {
    if (offset > frameLength_ || frameLength == 0)
        return 0;

    const auto framesToTrim = std::min(frameLength, frameLength_ - offset);
    const auto framesToMove = frameLength_ - (offset + framesToTrim);
    if (framesToMove) {
        const auto moveFromOffset = offset + framesToTrim;
        for (UInt32 i = 0; i < bufferList_->mNumberBuffers; ++i) {
            const auto data = reinterpret_cast<uintptr_t>(bufferList_->mBuffers[i].mData);
            std::memmove(reinterpret_cast<void *>(data + (offset * format_.mBytesPerFrame)),
                         reinterpret_cast<const void *>(data + (moveFromOffset * format_.mBytesPerFrame)),
                         framesToMove * format_.mBytesPerFrame);
        }
    }

    SetFrameLength(frameLength_ - framesToTrim);
    return framesToTrim;
}

UInt32 CXXCoreAudio::CAAudioBuffer::InsertSilence(UInt32 offset, UInt32 frameLength) noexcept {
    if (!(format_.IsFloat() || format_.IsSignedInteger()))
        // throw std::logic_error("Inserting silence for unsigned integer samples not supported");
        return 0;

    if (offset > frameLength_ || frameLength == 0)
        return 0;

    const auto framesToZero = std::min(frameCapacity_ - frameLength_, frameLength);
    const auto framesToMove = frameLength_ - offset;
    if (framesToMove) {
        const auto moveToOffset = offset + framesToZero;
        for (UInt32 i = 0; i < bufferList_->mNumberBuffers; ++i) {
            const auto data = reinterpret_cast<uintptr_t>(bufferList_->mBuffers[i].mData);
            std::memmove(reinterpret_cast<void *>(data + (moveToOffset * format_.mBytesPerFrame)),
                         reinterpret_cast<const void *>(data + (offset * format_.mBytesPerFrame)),
                         framesToMove * format_.mBytesPerFrame);
        }
    }

    if (framesToZero) {
        // For floating-point numbers this code is non-portable: the C standard doesn't require IEEE 754 compliance
        // However, setting all bits to 0 using memset() on macOS results in a floating-point value of 0
        for (UInt32 i = 0; i < bufferList_->mNumberBuffers; ++i) {
            auto s = reinterpret_cast<uintptr_t>(bufferList_->mBuffers[i].mData) + (offset * format_.mBytesPerFrame);
            std::memset(reinterpret_cast<void *>(s), 0, framesToZero * format_.mBytesPerFrame);
        }

        SetFrameLength(frameLength_ + framesToZero);
    }

    return framesToZero;
}

// MARK: AudioBufferList Management

bool CXXCoreAudio::CAAudioBuffer::adopt(AudioBufferList *bufferList, const AudioStreamBasicDescription& format,
                                        UInt32 frameCapacity, UInt32 frameLength) noexcept {
    if (!bufferList)
        return false;
    if (format.mBytesPerFrame == 0 || format.mChannelsPerFrame == 0)
        return false;
    if (frameCapacity == 0 || frameCapacity > (std::numeric_limits<UInt32>::max() / format.mBytesPerFrame))
        return false;
    if (frameLength > frameCapacity)
        return false;
    if (bufferList->mNumberBuffers != (format.mFormatFlags & kAudioFormatFlagIsNonInterleaved)
              ? format.mChannelsPerFrame
              : 1)
        return false;

    for (UInt32 i = 0; i < bufferList->mNumberBuffers; ++i) {
        if (bufferList->mBuffers[i].mDataByteSize != frameLength * format.mBytesPerFrame)
            return false;
    }

    std::free(std::exchange(bufferList_, bufferList));
    format_ = format;
    frameCapacity_ = frameCapacity;
    frameLength_ = frameLength;

    return true;
}

AudioBufferList *CXXCoreAudio::CAAudioBuffer::release() noexcept {
    format_.Reset();
    frameCapacity_ = 0;
    frameLength_ = 0;
    return std::exchange(bufferList_, nullptr);
}
