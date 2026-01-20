//
// SPDX-FileCopyrightText: 2014 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXCoreAudio
//

#pragma once

#import <CoreAudioTypes/CoreAudioTypes.h>
#import <CoreFoundation/CFString.h>
#ifdef __OBJC__
#import <AVFAudio/AVFAudio.h>
#import <Foundation/NSString.h>
#endif /* __OBJC__ */

#import <cassert>
#import <cstring>
#import <optional>

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
std::optional<CACommonPCMFormat> identifyCommonPCMFormat(const AudioStreamBasicDescription& streamDescription) noexcept;

/// Returns the name of the format described by an AudioStreamBasicDescription structure.
/// @note The caller is responsible for releasing the returned string.
CFStringRef _Nullable copyAudioStreamBasicDescriptionFormatName(
      const AudioStreamBasicDescription& streamDescription) noexcept CF_RETURNS_RETAINED;

/// Returns a string representation of the stream format described by an AudioStreamBasicDescription structure.
/// @note The caller is responsible for releasing the returned string.
CFStringRef _Nullable copyAudioStreamBasicDescriptionFormatDescription(
      const AudioStreamBasicDescription& streamDescription) noexcept CF_RETURNS_RETAINED;

#ifdef __OBJC__
/// Returns the name of the format described by an AudioStreamBasicDescription structure.
NSString *_Nullable audioStreamBasicDescriptionFormatName(
      const AudioStreamBasicDescription& streamDescription) noexcept;

/// Returns a string representation of the stream format described by an AudioStreamBasicDescription structure.
NSString *_Nullable audioStreamBasicDescriptionFormatDescription(
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
    [[nodiscard]] std::optional<CACommonPCMFormat> identifyCommonPCMFormat() const noexcept;

    /// Returns true if the kAudioFormatFlagIsNonInterleaved flag is set.
    [[nodiscard]] bool isNonInterleaved() const noexcept;

    /// Returns true if the kAudioFormatFlagIsNonInterleaved flag is clear.
    [[nodiscard]] bool isInterleaved() const noexcept;

    /// Returns the number of interleaved channels.
    [[nodiscard]] UInt32 interleavedChannelCount() const noexcept;

    /// Returns the number of channel streams.
    [[nodiscard]] UInt32 channelStreamCount() const noexcept;

    /// Returns the number of channels.
    [[nodiscard]] UInt32 channelCount() const noexcept;

    /// Returns true if mFormatID == kAudioFormatLinearPCM.
    [[nodiscard]] bool isPCM() const noexcept;

    /// Returns true if the kAudioFormatFlagIsBigEndian flag is set.
    [[nodiscard]] bool isBigEndian() const noexcept;

    /// Returns true if the kAudioFormatFlagIsBigEndian flag is clear.
    [[nodiscard]] bool isLittleEndian() const noexcept;

    /// Returns true if this format is native-endian.
    [[nodiscard]] bool isNativeEndian() const noexcept;

    /// Returns true if this format is linear PCM and the kAudioFormatFlagIsFloat flag is set.
    [[nodiscard]] bool isFloat() const noexcept;

    /// Returns true if this format is linear PCM and the kAudioFormatFlagIsFloat flag is clear.
    [[nodiscard]] bool isInteger() const noexcept;

    /// Returns true if this format is linear PCM and the kAudioFormatFlagIsSignedInteger flag is set.
    [[nodiscard]] bool isSignedInteger() const noexcept;

    /// Returns true if the kAudioFormatFlagIsPacked flag is set.
    [[nodiscard]] bool isPacked() const noexcept;

    /// Returns true if this format is implicitly packed.
    ///
    /// A format is implicitly packed when ((mBitsPerChannel / 8) * InterleavedChannelCount()) == mBytesPerFrame
    [[nodiscard]] bool isImplicitlyPacked() const noexcept;

    /// Returns true if this format is linear PCM and the sample bits do not occupy the entire channel.
    [[nodiscard]] bool isUnpackedPCM() const noexcept;

    /// Returns true if the kAudioFormatFlagIsAlignedHigh flag is set.
    [[nodiscard]] bool isAlignedHigh() const noexcept;

    /// Returns true if this format is unpacked linear PCM or if mBitsPerChannel is not a multiple of 8.
    [[nodiscard]] bool isUnaligned() const noexcept;

    /// Returns the number of fractional bits.
    [[nodiscard]] UInt32 fractionalBits() const noexcept;

    /// Returns true if this format is integer fixed-point linear PCM.
    [[nodiscard]] bool isFixedPoint() const noexcept;

    /// Returns true if the kAudioFormatFlagIsNonMixable flag is set.
    /// @note This flag is only used when interacting with HAL stream formats.
    [[nodiscard]] bool isNonMixable() const noexcept;

    /// Returns true if this format is linear PCM and the kAudioFormatFlagIsNonMixable flag is clear.
    /// @note This flag is only used when interacting with HAL stream formats.
    [[nodiscard]] bool isMixable() const noexcept;

    /// Returns the sample word size in bytes.
    [[nodiscard]] UInt32 sampleWordSize() const noexcept;

    /// Returns the byte size of frameCount audio frames.
    /// @note This is equivalent to frameCount * mBytesPerFrame.
    [[nodiscard]] UInt32 frameCountToByteSize(UInt32 frameCount) const noexcept;

    /// Returns the frame count of byteSize bytes.
    /// @note This is equivalent to byteSize / mBytesPerFrame.
    [[nodiscard]] UInt32 byteSizeToFrameCount(UInt32 byteSize) const noexcept;

    /// Returns the duration of a single packet in seconds.
    [[nodiscard]] double packetDuration() const noexcept;

    // MARK: Format transformation

    /// Sets format to the equivalent non-interleaved format of this.
    /// @note Fails for non-PCM formats.
    /// @return true on success, false otherwise.
    bool getNonInterleavedEquivalent(AudioStreamBasicDescription& format) const noexcept;

    /// Sets format to the equivalent interleaved format of this.
    /// @note Fails for non-PCM formats.
    bool getInterleavedEquivalent(AudioStreamBasicDescription& format) const noexcept;

    /// Sets format to the equivalent standard format of this.
    /// @note Fails for non-PCM formats.
    /// @return true on success, false otherwise.
    bool getStandardEquivalent(AudioStreamBasicDescription& format) const noexcept;

    /// Resets the stream description to the default state.
    void reset() noexcept;

    // MARK: Format Name and Description

    /// Returns the name of this format.
    ///
    /// This is the value of kAudioFormatProperty_FormatName.
    /// @note The caller is responsible for releasing the returned string
    [[nodiscard]] CFStringRef _Nullable copyFormatName() const noexcept CF_RETURNS_RETAINED;

    /// Returns a string representation of this format.
    /// @note The caller is responsible for releasing the returned string.
    [[nodiscard]] CFStringRef _Nullable copyFormatDescription() const noexcept CF_RETURNS_RETAINED;

#ifdef __OBJC__
    /// Returns an AVAudioFormat object initialized with this format and no channel layout.
    [[nodiscard]] operator AVAudioFormat *_Nullable() const noexcept;

    /// Returns the name of this format.
    ///
    /// This is the value of kAudioFormatProperty_FormatName.
    [[nodiscard]] NSString *_Nullable formatName() const noexcept;

    /// Returns a string representation of this format.
    [[nodiscard]] NSString *_Nullable formatDescription() const noexcept;
#endif /* __OBJC__ */
};

// MARK: - Implementation -

#ifdef __OBJC__
inline NSString *_Nullable audioStreamBasicDescriptionFormatName(
      const AudioStreamBasicDescription& streamDescription) noexcept {
    return (__bridge_transfer NSString *)copyAudioStreamBasicDescriptionFormatName(streamDescription);
}

inline NSString *_Nullable audioStreamBasicDescriptionFormatDescription(
      const AudioStreamBasicDescription& streamDescription) noexcept {
    return (__bridge_transfer NSString *)copyAudioStreamBasicDescriptionFormatDescription(streamDescription);
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

inline std::optional<CACommonPCMFormat> CAStreamDescription::identifyCommonPCMFormat() const noexcept {
    return CXXCoreAudio::identifyCommonPCMFormat(*this);
}

inline bool CAStreamDescription::isNonInterleaved() const noexcept {
    return (mFormatFlags & kAudioFormatFlagIsNonInterleaved) == kAudioFormatFlagIsNonInterleaved;
}

inline bool CAStreamDescription::isInterleaved() const noexcept {
    return (mFormatFlags & kAudioFormatFlagIsNonInterleaved) == 0;
}

inline UInt32 CAStreamDescription::interleavedChannelCount() const noexcept {
    return isInterleaved() ? mChannelsPerFrame : 1;
}

inline UInt32 CAStreamDescription::channelStreamCount() const noexcept {
    return isInterleaved() ? 1 : mChannelsPerFrame;
}

inline UInt32 CAStreamDescription::channelCount() const noexcept {
    return mChannelsPerFrame;
}

inline bool CAStreamDescription::isPCM() const noexcept {
    return mFormatID == kAudioFormatLinearPCM;
}

inline bool CAStreamDescription::isBigEndian() const noexcept {
    return (mFormatFlags & kAudioFormatFlagIsBigEndian) == kAudioFormatFlagIsBigEndian;
}

inline bool CAStreamDescription::isLittleEndian() const noexcept {
    return (mFormatFlags & kAudioFormatFlagIsBigEndian) == 0;
}

inline bool CAStreamDescription::isNativeEndian() const noexcept {
    return (mFormatFlags & kAudioFormatFlagIsBigEndian) == kAudioFormatFlagsNativeEndian;
}

inline bool CAStreamDescription::isFloat() const noexcept {
    return isPCM() && (mFormatFlags & kAudioFormatFlagIsFloat) == kAudioFormatFlagIsFloat;
}

inline bool CAStreamDescription::isInteger() const noexcept {
    return isPCM() && (mFormatFlags & kAudioFormatFlagIsFloat) == 0;
}

inline bool CAStreamDescription::isSignedInteger() const noexcept {
    return isPCM() && (mFormatFlags & kAudioFormatFlagIsSignedInteger) == kAudioFormatFlagIsSignedInteger;
}

inline bool CAStreamDescription::isPacked() const noexcept {
    return (mFormatFlags & kAudioFormatFlagIsPacked) == kAudioFormatFlagIsPacked;
}

inline bool CAStreamDescription::isImplicitlyPacked() const noexcept {
    return ((mBitsPerChannel / 8) * interleavedChannelCount()) == mBytesPerFrame;
}

inline bool CAStreamDescription::isUnpackedPCM() const noexcept {
    return isPCM() && (sampleWordSize() << 3) != mBitsPerChannel;
}

inline bool CAStreamDescription::isAlignedHigh() const noexcept {
    return (mFormatFlags & kAudioFormatFlagIsAlignedHigh) == kAudioFormatFlagIsAlignedHigh;
}

inline bool CAStreamDescription::isUnaligned() const noexcept {
    return isUnpackedPCM() || (mBitsPerChannel & 7) != 0;
}

inline UInt32 CAStreamDescription::fractionalBits() const noexcept {
    return (mFormatFlags & kLinearPCMFormatFlagsSampleFractionMask) >> kLinearPCMFormatFlagsSampleFractionShift;
}

inline bool CAStreamDescription::isFixedPoint() const noexcept {
    return isInteger() && fractionalBits() > 0;
}

inline bool CAStreamDescription::isNonMixable() const noexcept {
    return (mFormatFlags & kAudioFormatFlagIsNonMixable) == kAudioFormatFlagIsNonMixable;
}

inline bool CAStreamDescription::isMixable() const noexcept {
    return isPCM() && (mFormatFlags & kAudioFormatFlagIsNonMixable) == 0;
}

inline UInt32 CAStreamDescription::sampleWordSize() const noexcept {
    const auto interleavedChannels = interleavedChannelCount();
    if (interleavedChannels == 0 || mBytesPerFrame % interleavedChannels != 0)
        return 0;
    return mBytesPerFrame / interleavedChannels;
}

inline UInt32 CAStreamDescription::frameCountToByteSize(UInt32 frameCount) const noexcept {
    return frameCount * mBytesPerFrame;
}

inline UInt32 CAStreamDescription::byteSizeToFrameCount(UInt32 byteSize) const noexcept {
    assert(mBytesPerFrame > 0);
    return byteSize / mBytesPerFrame;
}

inline double CAStreamDescription::packetDuration() const noexcept {
    assert(mSampleRate > 0);
    return (1 / mSampleRate) * mFramesPerPacket;
}

// MARK: Format transformation

inline void CAStreamDescription::reset() noexcept {
    std::memset(this, 0, sizeof(AudioStreamBasicDescription));
}

// MARK: Format Name and Description

inline CFStringRef _Nullable CAStreamDescription::copyFormatName() const noexcept {
    return copyAudioStreamBasicDescriptionFormatName(*this);
}

inline CFStringRef _Nullable CAStreamDescription::copyFormatDescription() const noexcept {
    return copyAudioStreamBasicDescriptionFormatDescription(*this);
}

#ifdef __OBJC__
inline CAStreamDescription::operator AVAudioFormat *_Nullable() const noexcept {
    return [[AVAudioFormat alloc] initWithStreamDescription:this];
}

inline NSString *_Nullable CAStreamDescription::formatName() const noexcept {
    return (__bridge_transfer NSString *)copyFormatName();
}

inline NSString *_Nullable CAStreamDescription::formatDescription() const noexcept {
    return (__bridge_transfer NSString *)copyFormatDescription();
}
#endif /* __OBJC__ */

} /* namespace CXXCoreAudio */
