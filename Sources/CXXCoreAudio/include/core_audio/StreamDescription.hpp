//
// SPDX-FileCopyrightText: 2014 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXCoreAudio
//

#pragma once

#include <cf/CFRef.hpp>

#include <CoreAudioTypes/CoreAudioTypes.h>
#include <CoreFoundation/CFString.h>

#ifdef __OBJC__
#import <AVFAudio/AVFAudio.h>
#import <Foundation/NSString.h>
#endif /* __OBJC__ */

#include <cassert>
#include <cstring>
#include <optional>

namespace core_audio {

/// Common PCM audio formats.
enum class CommonPCMFormat {
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
std::optional<CommonPCMFormat> identifyCommonPCMFormat(const AudioStreamBasicDescription &streamDescription) noexcept;

/// Returns the name of the format described by an AudioStreamBasicDescription structure.
/// @note The caller is responsible for releasing the returned string.
cf::CFString copyAudioStreamBasicDescriptionFormatName(
        const AudioStreamBasicDescription &streamDescription) noexcept;

/// Returns a string representation of the stream format described by an AudioStreamBasicDescription structure.
/// @note The caller is responsible for releasing the returned string.
cf::CFString copyAudioStreamBasicDescriptionFormatDescription(
        const AudioStreamBasicDescription &streamDescription) noexcept;

#ifdef __OBJC__
/// Returns the name of the format described by an AudioStreamBasicDescription structure.
NSString *_Nullable audioStreamBasicDescriptionFormatName(
        const AudioStreamBasicDescription &streamDescription) noexcept;

/// Returns a string representation of the stream format described by an AudioStreamBasicDescription structure.
NSString *_Nullable audioStreamBasicDescriptionFormatDescription(
        const AudioStreamBasicDescription &streamDescription) noexcept;
#endif /* __OBJC__ */

/// A class extending the functionality of an AudioStreamBasicDescription structure.
struct StreamDescription final : public AudioStreamBasicDescription {
    // MARK: Construction and Destruction

    /// Creates an empty stream description.
    StreamDescription() noexcept = default;

    /// Creates a stream description for the specified common PCM format.
    StreamDescription(CommonPCMFormat commonPCMFormat, Float64 sampleRate, UInt32 channelsPerFrame,
                      bool isInterleaved) noexcept;

    /// Creates a copy of an existing stream description.
    StreamDescription(const StreamDescription &other) noexcept = default;

    /// Creates a stream description copied from an AudioStreamBasicDescription.
    StreamDescription(const AudioStreamBasicDescription &other) noexcept;

    /// Assignment operator
    StreamDescription &operator=(const StreamDescription &other) noexcept = default;

    /// Assignment operator
    StreamDescription &operator=(const AudioStreamBasicDescription &other) noexcept;

    /// Destructor
    ~StreamDescription() noexcept = default;

    // MARK: Comparison

    /// Returns true if other is equal to this.
    [[nodiscard]] bool operator==(const AudioStreamBasicDescription &other) const noexcept;

    /// Returns true if other is not equal to this.
    [[nodiscard]] bool operator!=(const AudioStreamBasicDescription &other) const noexcept;

    // MARK: Format Information

    /// Returns the common PCM format described by this stream description or std::nullopt if none.
    [[nodiscard]] std::optional<CommonPCMFormat> identifyCommonPCMFormat() const noexcept;

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
    bool getNonInterleavedEquivalent(AudioStreamBasicDescription &format) const noexcept;

    /// Sets format to the equivalent interleaved format of this.
    /// @note Fails for non-PCM formats.
    bool getInterleavedEquivalent(AudioStreamBasicDescription &format) const noexcept;

    /// Sets format to the equivalent standard format of this.
    /// @note Fails for non-PCM formats.
    /// @return true on success, false otherwise.
    bool getStandardEquivalent(AudioStreamBasicDescription &format) const noexcept;

    /// Resets the stream description to the default state.
    void reset() noexcept;

    // MARK: Format Name and Description

    /// Returns the name of this format.
    ///
    /// This is the value of kAudioFormatProperty_FormatName.
    /// @note The caller is responsible for releasing the returned string
    [[nodiscard]] cf::CFString copyFormatName() const noexcept;

    /// Returns a string representation of this format.
    /// @note The caller is responsible for releasing the returned string.
    [[nodiscard]] cf::CFString copyFormatDescription() const noexcept;

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
        const AudioStreamBasicDescription &streamDescription) noexcept {
    auto formatName = copyAudioStreamBasicDescriptionFormatName(streamDescription);
    return (__bridge_transfer NSString *)formatName.leak();
}

inline NSString *_Nullable audioStreamBasicDescriptionFormatDescription(
      const AudioStreamBasicDescription &streamDescription) noexcept {
    auto formatDescription = copyAudioStreamBasicDescriptionFormatDescription(streamDescription);
    return (__bridge_transfer NSString *)formatDescription.leak();
}
#endif /* __OBJC__ */

// MARK: Construction and Destruction

inline StreamDescription::StreamDescription(const AudioStreamBasicDescription &other) noexcept
    : AudioStreamBasicDescription(other) {}

inline StreamDescription &StreamDescription::operator=(const AudioStreamBasicDescription &other) noexcept {
    AudioStreamBasicDescription::operator=(other);
    return *this;
}

// MARK: Comparison

/// Returns true if other is equal to this.
inline bool StreamDescription::operator==(const AudioStreamBasicDescription &other) const noexcept {
    return !std::memcmp(this, &other, sizeof(AudioStreamBasicDescription));
}

inline bool StreamDescription::operator!=(const AudioStreamBasicDescription &other) const noexcept {
    return !operator==(other);
}

// MARK: Format Information

inline std::optional<CommonPCMFormat> StreamDescription::identifyCommonPCMFormat() const noexcept {
    return core_audio::identifyCommonPCMFormat(*this);
}

inline bool StreamDescription::isNonInterleaved() const noexcept {
    return (mFormatFlags & kAudioFormatFlagIsNonInterleaved) == kAudioFormatFlagIsNonInterleaved;
}

inline bool StreamDescription::isInterleaved() const noexcept {
    return (mFormatFlags & kAudioFormatFlagIsNonInterleaved) == 0;
}

inline UInt32 StreamDescription::interleavedChannelCount() const noexcept {
    return isInterleaved() ? mChannelsPerFrame : 1;
}

inline UInt32 StreamDescription::channelStreamCount() const noexcept { return isInterleaved() ? 1 : mChannelsPerFrame; }

inline UInt32 StreamDescription::channelCount() const noexcept { return mChannelsPerFrame; }

inline bool StreamDescription::isPCM() const noexcept { return mFormatID == kAudioFormatLinearPCM; }

inline bool StreamDescription::isBigEndian() const noexcept {
    return (mFormatFlags & kAudioFormatFlagIsBigEndian) == kAudioFormatFlagIsBigEndian;
}

inline bool StreamDescription::isLittleEndian() const noexcept {
    return (mFormatFlags & kAudioFormatFlagIsBigEndian) == 0;
}

inline bool StreamDescription::isNativeEndian() const noexcept {
    return (mFormatFlags & kAudioFormatFlagIsBigEndian) == kAudioFormatFlagsNativeEndian;
}

inline bool StreamDescription::isFloat() const noexcept {
    return isPCM() && (mFormatFlags & kAudioFormatFlagIsFloat) == kAudioFormatFlagIsFloat;
}

inline bool StreamDescription::isInteger() const noexcept {
    return isPCM() && (mFormatFlags & kAudioFormatFlagIsFloat) == 0;
}

inline bool StreamDescription::isSignedInteger() const noexcept {
    return isPCM() && (mFormatFlags & kAudioFormatFlagIsSignedInteger) == kAudioFormatFlagIsSignedInteger;
}

inline bool StreamDescription::isPacked() const noexcept {
    return (mFormatFlags & kAudioFormatFlagIsPacked) == kAudioFormatFlagIsPacked;
}

inline bool StreamDescription::isImplicitlyPacked() const noexcept {
    return ((mBitsPerChannel / 8) * interleavedChannelCount()) == mBytesPerFrame;
}

inline bool StreamDescription::isUnpackedPCM() const noexcept {
    return isPCM() && (sampleWordSize() << 3) != mBitsPerChannel;
}

inline bool StreamDescription::isAlignedHigh() const noexcept {
    return (mFormatFlags & kAudioFormatFlagIsAlignedHigh) == kAudioFormatFlagIsAlignedHigh;
}

inline bool StreamDescription::isUnaligned() const noexcept { return isUnpackedPCM() || (mBitsPerChannel & 7) != 0; }

inline UInt32 StreamDescription::fractionalBits() const noexcept {
    return (mFormatFlags & kLinearPCMFormatFlagsSampleFractionMask) >> kLinearPCMFormatFlagsSampleFractionShift;
}

inline bool StreamDescription::isFixedPoint() const noexcept { return isInteger() && fractionalBits() > 0; }

inline bool StreamDescription::isNonMixable() const noexcept {
    return (mFormatFlags & kAudioFormatFlagIsNonMixable) == kAudioFormatFlagIsNonMixable;
}

inline bool StreamDescription::isMixable() const noexcept {
    return isPCM() && (mFormatFlags & kAudioFormatFlagIsNonMixable) == 0;
}

inline UInt32 StreamDescription::sampleWordSize() const noexcept {
    const auto interleavedChannels = interleavedChannelCount();
    if (interleavedChannels == 0 || mBytesPerFrame % interleavedChannels != 0) {
        return 0;
    }
    return mBytesPerFrame / interleavedChannels;
}

inline UInt32 StreamDescription::frameCountToByteSize(UInt32 frameCount) const noexcept {
    return frameCount * mBytesPerFrame;
}

inline UInt32 StreamDescription::byteSizeToFrameCount(UInt32 byteSize) const noexcept {
    assert(mBytesPerFrame > 0);
    return byteSize / mBytesPerFrame;
}

inline double StreamDescription::packetDuration() const noexcept {
    assert(mSampleRate > 0);
    return (1 / mSampleRate) * mFramesPerPacket;
}

// MARK: Format transformation

inline void StreamDescription::reset() noexcept { std::memset(this, 0, sizeof(AudioStreamBasicDescription)); }

// MARK: Format Name and Description

inline cf::CFString StreamDescription::copyFormatName() const noexcept {
    return copyAudioStreamBasicDescriptionFormatName(*this);
}

inline cf::CFString StreamDescription::copyFormatDescription() const noexcept {
    return copyAudioStreamBasicDescriptionFormatDescription(*this);
}

#ifdef __OBJC__
inline StreamDescription::operator AVAudioFormat *_Nullable() const noexcept {
    return [[AVAudioFormat alloc] initWithStreamDescription:this];
}

inline NSString *_Nullable StreamDescription::formatName() const noexcept {
    return audioStreamBasicDescriptionFormatName(*this);
}

inline NSString *_Nullable StreamDescription::formatDescription() const noexcept {
    return audioStreamBasicDescriptionFormatDescription(*this);
}
#endif /* __OBJC__ */

} /* namespace core_audio */
