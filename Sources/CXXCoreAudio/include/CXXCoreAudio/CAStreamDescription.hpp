//
// SPDX-FileCopyrightText: 2014 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXCoreAudio
//

#pragma once

#include <CoreAudioTypes/CoreAudioTypes.h>
#include <CoreFoundation/CFString.h>
#ifdef __OBJC__
#include <AVFAudio/AVFAudio.h>
#include <Foundation/NSString.h>
#endif /* __OBJC__ */

#include <cassert>
#include <cstring>
#include <optional>

namespace CXXCoreAudio {

/// Common PCM audio formats.
enum class CACommonPCMFormat {
    /// Native-endian float.
    float32,
    /// Native-endian double.
    float64,
    /// Native-endian int16_t.
    int16,
    /// Native-endian int32_t.
    int32,
};

// MARK: AudioStreamBasicDescription Helper Functions

/// Returns the common PCM format described by an AudioStreamBasicDescription structure or std::nullopt if none.
std::optional<CACommonPCMFormat> IdentifyCommonPCMFormat(const AudioStreamBasicDescription& streamDescription) noexcept;

/// Returns the name of the format described by an AudioStreamBasicDescription structure.
/// @note The caller is responsible for releasing the returned string.
CFStringRef _Nullable CopyAudioStreamBasicDescriptionFormatName(
      const AudioStreamBasicDescription& streamDescription) noexcept CF_RETURNS_RETAINED;

/// Returns a string representation of the stream format described by an AudioStreamBasicDescription structure.
/// @note The caller is responsible for releasing the returned string.
CFStringRef _Nullable CopyAudioStreamBasicDescriptionFormatDescription(
      const AudioStreamBasicDescription& streamDescription) noexcept CF_RETURNS_RETAINED;

#ifdef __OBJC__
/// Returns the name of the format described by an AudioStreamBasicDescription structure.
NSString *_Nullable AudioStreamBasicDescriptionFormatName(
      const AudioStreamBasicDescription& streamDescription) noexcept;

/// Returns a string representation of the stream format described by an AudioStreamBasicDescription structure.
NSString *_Nullable AudioStreamBasicDescriptionFormatDescription(
      const AudioStreamBasicDescription& streamDescription) noexcept;
#endif /* __OBJC__ */

/// A class extending the functionality of an AudioStreamBasicDescription structure.
struct CAStreamDescription final : public AudioStreamBasicDescription {
    // MARK: Creation and Destruction

    /// Creates an empty stream description.
    CAStreamDescription() noexcept = default;

    /// Creates a stream description for the specified common PCM format.
    CAStreamDescription(CACommonPCMFormat commonPCMFormat, Float64 sampleRate, UInt32 channelsPerFrame,
                        bool isInterleaved) noexcept;

    /// Creates a copy of an existing stream description.
    CAStreamDescription(const CAStreamDescription& other) noexcept = default;

    /// Creates a stream description copied from an AudioStreamBasicDescription.
    CAStreamDescription(const AudioStreamBasicDescription& other) noexcept;

    /// Assignment operator
    CAStreamDescription& operator=(const CAStreamDescription& other) noexcept = default;

    /// Assignment operator
    CAStreamDescription& operator=(const AudioStreamBasicDescription& other) noexcept;

    /// Destructor
    ~CAStreamDescription() noexcept = default;

    // MARK: Comparison

    /// Returns true if other is equal to this.
    [[nodiscard]] bool operator==(const AudioStreamBasicDescription& other) const noexcept;

    /// Returns true if other is not equal to this.
    [[nodiscard]] bool operator!=(const AudioStreamBasicDescription& other) const noexcept;

    // MARK: Format Information

    /// Returns the common PCM format described by this stream description or std::nullopt if none.
    [[nodiscard]] std::optional<CACommonPCMFormat> IdentifyCommonPCMFormat() const noexcept;

    /// Returns true if the kAudioFormatFlagIsNonInterleaved flag is set.
    [[nodiscard]] bool IsNonInterleaved() const noexcept;

    /// Returns true if the kAudioFormatFlagIsNonInterleaved flag is clear.
    [[nodiscard]] bool IsInterleaved() const noexcept;

    /// Returns the number of interleaved channels.
    [[nodiscard]] UInt32 InterleavedChannelCount() const noexcept;

    /// Returns the number of channel streams.
    [[nodiscard]] UInt32 ChannelStreamCount() const noexcept;

    /// Returns the number of channels.
    [[nodiscard]] UInt32 ChannelCount() const noexcept;

    /// Returns true if mFormatID == kAudioFormatLinearPCM.
    [[nodiscard]] bool IsPCM() const noexcept;

    /// Returns true if the kAudioFormatFlagIsBigEndian flag is set.
    [[nodiscard]] bool IsBigEndian() const noexcept;

    /// Returns true if the kAudioFormatFlagIsBigEndian flag is clear.
    [[nodiscard]] bool IsLittleEndian() const noexcept;

    /// Returns true if this format is native-endian.
    [[nodiscard]] bool IsNativeEndian() const noexcept;

    /// Returns true if this format is linear PCM and the kAudioFormatFlagIsFloat flag is set.
    [[nodiscard]] bool IsFloat() const noexcept;

    /// Returns true if this format is linear PCM and the kAudioFormatFlagIsFloat flag is clear.
    [[nodiscard]] bool IsInteger() const noexcept;

    /// Returns true if this format is linear PCM and the kAudioFormatFlagIsSignedInteger flag is set.
    [[nodiscard]] bool IsSignedInteger() const noexcept;

    /// Returns true if the kAudioFormatFlagIsPacked flag is set.
    [[nodiscard]] bool IsPacked() const noexcept;

    /// Returns true if this format is implicitly packed.
    ///
    /// A format is implicitly packed when ((mBitsPerChannel / 8) * InterleavedChannelCount()) == mBytesPerFrame
    [[nodiscard]] bool IsImplicitlyPacked() const noexcept;

    /// Returns true if this format is linear PCM and the sample bits do not occupy the entire channel.
    [[nodiscard]] bool IsUnpackedPCM() const noexcept;

    /// Returns true if the kAudioFormatFlagIsAlignedHigh flag is set.
    [[nodiscard]] bool IsAlignedHigh() const noexcept;

    /// Returns true if this format is unpacked linear PCM or if mBitsPerChannel is not a multiple of 8.
    [[nodiscard]] bool IsUnaligned() const noexcept;

    /// Returns the number of fractional bits.
    [[nodiscard]] UInt32 FractionalBits() const noexcept;

    /// Returns true if this format is integer fixed-point linear PCM.
    [[nodiscard]] bool IsFixedPoint() const noexcept;

    /// Returns true if the kAudioFormatFlagIsNonMixable flag is set.
    /// @note This flag is only used when interacting with HAL stream formats.
    [[nodiscard]] bool IsNonMixable() const noexcept;

    /// Returns true if this format is linear PCM and the kAudioFormatFlagIsNonMixable flag is clear.
    /// @note This flag is only used when interacting with HAL stream formats.
    [[nodiscard]] bool IsMixable() const noexcept;

    /// Returns the sample word size in bytes.
    [[nodiscard]] UInt32 SampleWordSize() const noexcept;

    /// Returns the byte size of frameCount audio frames.
    /// @note This is equivalent to frameCount * mBytesPerFrame.
    [[nodiscard]] UInt32 FrameCountToByteSize(UInt32 frameCount) const noexcept;

    /// Returns the frame count of byteSize bytes.
    /// @note This is equivalent to byteSize / mBytesPerFrame.
    [[nodiscard]] UInt32 ByteSizeToFrameCount(UInt32 byteSize) const noexcept;

    /// Returns the duration of a single packet in seconds.
    [[nodiscard]] double PacketDuration() const noexcept;

    // MARK: Format transformation

    /// Sets format to the equivalent non-interleaved format of this.
    /// @note Fails for non-PCM formats.
    /// @return true on success, false otherwise.
    bool GetNonInterleavedEquivalent(AudioStreamBasicDescription& format) const noexcept;

    /// Sets format to the equivalent interleaved format of this.
    /// @note Fails for non-PCM formats.
    bool GetInterleavedEquivalent(AudioStreamBasicDescription& format) const noexcept;

    /// Sets format to the equivalent standard format of this.
    /// @note Fails for non-PCM formats.
    /// @return true on success, false otherwise.
    bool GetStandardEquivalent(AudioStreamBasicDescription& format) const noexcept;

    /// Resets the stream description to the default state.
    void Reset() noexcept;

    // MARK: Format Name and Description

    /// Returns the name of this format.
    ///
    /// This is the value of kAudioFormatProperty_FormatName.
    /// @note The caller is responsible for releasing the returned string
    [[nodiscard]] CFStringRef _Nullable CopyFormatName() const noexcept CF_RETURNS_RETAINED;

    /// Returns a string representation of this format.
    /// @note The caller is responsible for releasing the returned string.
    [[nodiscard]] CFStringRef _Nullable CopyFormatDescription() const noexcept CF_RETURNS_RETAINED;

#ifdef __OBJC__
    /// Returns an AVAudioFormat object initialized with this format and no channel layout.
    [[nodiscard]] operator AVAudioFormat *_Nullable() const noexcept;

    /// Returns the name of this format.
    ///
    /// This is the value of kAudioFormatProperty_FormatName.
    [[nodiscard]] NSString *_Nullable FormatName() const noexcept;

    /// Returns a string representation of this format.
    [[nodiscard]] NSString *_Nullable FormatDescription() const noexcept;
#endif /* __OBJC__ */
};

// MARK: - Implementation -

#ifdef __OBJC__
inline NSString *_Nullable AudioStreamBasicDescriptionFormatName(
      const AudioStreamBasicDescription& streamDescription) noexcept {
    return (__bridge_transfer NSString *)CopyAudioStreamBasicDescriptionFormatName(streamDescription);
}

inline NSString *_Nullable AudioStreamBasicDescriptionFormatDescription(
      const AudioStreamBasicDescription& streamDescription) noexcept {
    return (__bridge_transfer NSString *)CopyAudioStreamBasicDescriptionFormatDescription(streamDescription);
}
#endif /* __OBJC__ */

// MARK: Creation and Destruction

inline CAStreamDescription::CAStreamDescription(const AudioStreamBasicDescription& other) noexcept
  : AudioStreamBasicDescription(other) {}

inline CAStreamDescription& CAStreamDescription::operator=(const AudioStreamBasicDescription& other) noexcept {
    AudioStreamBasicDescription::operator=(other);
    return *this;
}

// MARK: Comparison

/// Returns true if other is equal to this.
inline bool CAStreamDescription::operator==(const AudioStreamBasicDescription& other) const noexcept {
    return !std::memcmp(this, &other, sizeof(AudioStreamBasicDescription));
}

inline bool CAStreamDescription::operator!=(const AudioStreamBasicDescription& other) const noexcept {
    return !operator==(other);
}

// MARK: Format Information

inline std::optional<CACommonPCMFormat> CAStreamDescription::IdentifyCommonPCMFormat() const noexcept {
    return CXXCoreAudio::IdentifyCommonPCMFormat(*this);
}

inline bool CAStreamDescription::IsNonInterleaved() const noexcept {
    return (mFormatFlags & kAudioFormatFlagIsNonInterleaved) == kAudioFormatFlagIsNonInterleaved;
}

inline bool CAStreamDescription::IsInterleaved() const noexcept {
    return (mFormatFlags & kAudioFormatFlagIsNonInterleaved) == 0;
}

inline UInt32 CAStreamDescription::InterleavedChannelCount() const noexcept {
    return IsInterleaved() ? mChannelsPerFrame : 1;
}

inline UInt32 CAStreamDescription::ChannelStreamCount() const noexcept {
    return IsInterleaved() ? 1 : mChannelsPerFrame;
}

inline UInt32 CAStreamDescription::ChannelCount() const noexcept {
    return mChannelsPerFrame;
}

inline bool CAStreamDescription::IsPCM() const noexcept {
    return mFormatID == kAudioFormatLinearPCM;
}

inline bool CAStreamDescription::IsBigEndian() const noexcept {
    return (mFormatFlags & kAudioFormatFlagIsBigEndian) == kAudioFormatFlagIsBigEndian;
}

inline bool CAStreamDescription::IsLittleEndian() const noexcept {
    return (mFormatFlags & kAudioFormatFlagIsBigEndian) == 0;
}

inline bool CAStreamDescription::IsNativeEndian() const noexcept {
    return (mFormatFlags & kAudioFormatFlagIsBigEndian) == kAudioFormatFlagsNativeEndian;
}

inline bool CAStreamDescription::IsFloat() const noexcept {
    return IsPCM() && (mFormatFlags & kAudioFormatFlagIsFloat) == kAudioFormatFlagIsFloat;
}

inline bool CAStreamDescription::IsInteger() const noexcept {
    return IsPCM() && (mFormatFlags & kAudioFormatFlagIsFloat) == 0;
}

inline bool CAStreamDescription::IsSignedInteger() const noexcept {
    return IsPCM() && (mFormatFlags & kAudioFormatFlagIsSignedInteger) == kAudioFormatFlagIsSignedInteger;
}

inline bool CAStreamDescription::IsPacked() const noexcept {
    return (mFormatFlags & kAudioFormatFlagIsPacked) == kAudioFormatFlagIsPacked;
}

inline bool CAStreamDescription::IsImplicitlyPacked() const noexcept {
    return ((mBitsPerChannel / 8) * InterleavedChannelCount()) == mBytesPerFrame;
}

inline bool CAStreamDescription::IsUnpackedPCM() const noexcept {
    return IsPCM() && (SampleWordSize() << 3) != mBitsPerChannel;
}

inline bool CAStreamDescription::IsAlignedHigh() const noexcept {
    return (mFormatFlags & kAudioFormatFlagIsAlignedHigh) == kAudioFormatFlagIsAlignedHigh;
}

inline bool CAStreamDescription::IsUnaligned() const noexcept {
    return IsUnpackedPCM() || (mBitsPerChannel & 7) != 0;
}

inline UInt32 CAStreamDescription::FractionalBits() const noexcept {
    return (mFormatFlags & kLinearPCMFormatFlagsSampleFractionMask) >> kLinearPCMFormatFlagsSampleFractionShift;
}

inline bool CAStreamDescription::IsFixedPoint() const noexcept {
    return IsInteger() && FractionalBits() > 0;
}

inline bool CAStreamDescription::IsNonMixable() const noexcept {
    return (mFormatFlags & kAudioFormatFlagIsNonMixable) == kAudioFormatFlagIsNonMixable;
}

inline bool CAStreamDescription::IsMixable() const noexcept {
    return IsPCM() && (mFormatFlags & kAudioFormatFlagIsNonMixable) == 0;
}

inline UInt32 CAStreamDescription::SampleWordSize() const noexcept {
    const auto interleavedChannelCount = InterleavedChannelCount();
    if (interleavedChannelCount == 0 || mBytesPerFrame % interleavedChannelCount != 0) {
        return 0;
    }
    return mBytesPerFrame / interleavedChannelCount;
}

inline UInt32 CAStreamDescription::FrameCountToByteSize(UInt32 frameCount) const noexcept {
    return frameCount * mBytesPerFrame;
}

inline UInt32 CAStreamDescription::ByteSizeToFrameCount(UInt32 byteSize) const noexcept {
    assert(mBytesPerFrame > 0);
    return byteSize / mBytesPerFrame;
}

inline double CAStreamDescription::PacketDuration() const noexcept {
    assert(mSampleRate > 0);
    return (1 / mSampleRate) * mFramesPerPacket;
}

// MARK: Format transformation

inline void CAStreamDescription::Reset() noexcept {
    std::memset(this, 0, sizeof(AudioStreamBasicDescription));
}

// MARK: Format Name and Description

inline CFStringRef _Nullable CAStreamDescription::CopyFormatName() const noexcept {
    return CopyAudioStreamBasicDescriptionFormatName(*this);
}

inline CFStringRef _Nullable CAStreamDescription::CopyFormatDescription() const noexcept {
    return CopyAudioStreamBasicDescriptionFormatDescription(*this);
}

#ifdef __OBJC__
inline CAStreamDescription::operator AVAudioFormat *_Nullable() const noexcept {
    return [[AVAudioFormat alloc] initWithStreamDescription:this];
}

inline NSString *_Nullable CAStreamDescription::FormatName() const noexcept {
    return (__bridge_transfer NSString *)CopyFormatName();
}

inline NSString *_Nullable CAStreamDescription::FormatDescription() const noexcept {
    return (__bridge_transfer NSString *)CopyFormatDescription();
}
#endif /* __OBJC__ */

} /* namespace CXXCoreAudio */
