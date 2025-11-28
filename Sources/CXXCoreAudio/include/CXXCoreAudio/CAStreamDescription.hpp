//
// Copyright © 2014-2025 Stephen F. Booth
// Part of https://github.com/sbooth/CXXCoreAudio
// MIT license
//

#pragma once

#import <cstring>
#import <optional>

#import <CoreAudioTypes/CoreAudioTypes.h>
#import <CoreFoundation/CFString.h>

#ifdef __OBJC__
#import <AVFAudio/AVFAudio.h>
#import <Foundation/NSString.h>
#endif /* __OBJC__ */

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
CFStringRef _Nullable CopyAudioStreamBasicDescriptionFormatName(const AudioStreamBasicDescription& streamDescription) noexcept CF_RETURNS_RETAINED;

/// Returns a string representation of the stream format described by an AudioStreamBasicDescription structure.
/// @note The caller is responsible for releasing the returned string.
CFStringRef _Nullable CopyAudioStreamBasicDescriptionFormatDescription(const AudioStreamBasicDescription& streamDescription) noexcept CF_RETURNS_RETAINED;

#ifdef __OBJC__
/// Returns the name of the format described by an AudioStreamBasicDescription structure.
inline NSString * _Nullable AudioStreamBasicDescriptionFormatName(const AudioStreamBasicDescription& streamDescription) noexcept
{
	return (__bridge_transfer NSString *)CopyAudioStreamBasicDescriptionFormatName(streamDescription);
}

/// Returns a string representation of the stream format described by an AudioStreamBasicDescription structure.
inline NSString * _Nullable AudioStreamBasicDescriptionFormatDescription(const AudioStreamBasicDescription& streamDescription) noexcept
{
	return (__bridge_transfer NSString *)CopyAudioStreamBasicDescriptionFormatDescription(streamDescription);
}
#endif /* __OBJC__ */

/// A class extending the functionality of an AudioStreamBasicDescription structure.
struct CAStreamDescription final : public AudioStreamBasicDescription {
	// MARK: Creation and Destruction

	/// Creates an empty stream description.
	CAStreamDescription() noexcept = default;

	/// Creates a stream description for the specified common PCM format.
	CAStreamDescription(CACommonPCMFormat commonPCMFormat, Float64 sampleRate, UInt32 channelsPerFrame, bool isInterleaved) noexcept;

	/// Creates a copy of an existing stream description.
	CAStreamDescription(const CAStreamDescription& other) noexcept = default;

	/// Creates a stream description copied from an AudioStreamBasicDescription.
	CAStreamDescription(const AudioStreamBasicDescription& other) noexcept
	: AudioStreamBasicDescription{other}
	{}

	/// Assignment operator
	CAStreamDescription& operator=(const CAStreamDescription& other) noexcept = default;

	/// Assignment operator
	CAStreamDescription& operator=(const AudioStreamBasicDescription& other) noexcept
	{
		AudioStreamBasicDescription::operator=(other);
		return *this;
	}

//	CAStreamDescription(CAStreamDescription&&) = delete;
//	CAStreamDescription& operator=(CAStreamDescription&&) = delete;

	/// Destructor
	~CAStreamDescription() noexcept = default;

	// MARK: Comparison

	/// Returns true if other is equal to this.
	bool operator==(const AudioStreamBasicDescription& other) const noexcept
	{
		return !std::memcmp(this, &other, sizeof(AudioStreamBasicDescription));
	}

	/// Returns true if other is not equal to this.
	bool operator!=(const AudioStreamBasicDescription& other) const noexcept
	{
		return !operator==(other);
	}

	// MARK: Format Information

	/// Returns the common PCM format described by this stream description or std::nullopt if none.
	std::optional<CACommonPCMFormat> IdentifyCommonPCMFormat() const noexcept
	{
		return CXXCoreAudio::IdentifyCommonPCMFormat(*this);
	}

	/// Returns true if the kAudioFormatFlagIsNonInterleaved flag is set.
	bool IsNonInterleaved() const noexcept
	{
		return (mFormatFlags & kAudioFormatFlagIsNonInterleaved) == kAudioFormatFlagIsNonInterleaved;
	}

	/// Returns true if the kAudioFormatFlagIsNonInterleaved flag is clear.
	bool IsInterleaved() const noexcept
	{
		return (mFormatFlags & kAudioFormatFlagIsNonInterleaved) == 0;
	}

	/// Returns the number of interleaved channels.
	UInt32 InterleavedChannelCount() const noexcept
	{
		return IsInterleaved() ? mChannelsPerFrame : 1;
	}

	/// Returns the number of channel streams.
	UInt32 ChannelStreamCount() const noexcept
	{
		return IsInterleaved() ? 1 : mChannelsPerFrame;
	}

	/// Returns the number of channels.
	UInt32 ChannelCount() const noexcept
	{
		return mChannelsPerFrame;
	}

	/// Returns true if mFormatID == kAudioFormatLinearPCM.
	bool IsPCM() const noexcept
	{
		return mFormatID == kAudioFormatLinearPCM;
	}

	/// Returns true if the kAudioFormatFlagIsBigEndian flag is set.
	bool IsBigEndian() const noexcept
	{
		return (mFormatFlags & kAudioFormatFlagIsBigEndian) == kAudioFormatFlagIsBigEndian;
	}

	/// Returns true if the kAudioFormatFlagIsBigEndian flag is clear.
	bool IsLittleEndian() const noexcept
	{
		return (mFormatFlags & kAudioFormatFlagIsBigEndian) == 0;
	}

	/// Returns true if this format is native-endian.
	bool IsNativeEndian() const noexcept
	{
		return (mFormatFlags & kAudioFormatFlagIsBigEndian) == kAudioFormatFlagsNativeEndian;
	}

	/// Returns true if this format is linear PCM and the kAudioFormatFlagIsFloat flag is set.
	bool IsFloat() const noexcept
	{
		return IsPCM() && (mFormatFlags & kAudioFormatFlagIsFloat) == kAudioFormatFlagIsFloat;
	}

	/// Returns true if this format is linear PCM and the kAudioFormatFlagIsFloat flag is clear.
	bool IsInteger() const noexcept
	{
		return IsPCM() && (mFormatFlags & kAudioFormatFlagIsFloat) == 0;
	}

	/// Returns true if this format is linear PCM and the kAudioFormatFlagIsSignedInteger flag is set.
	bool IsSignedInteger() const noexcept
	{
		return IsPCM() && (mFormatFlags & kAudioFormatFlagIsSignedInteger) == kAudioFormatFlagIsSignedInteger;
	}

	/// Returns true if the kAudioFormatFlagIsPacked flag is set.
	bool IsPacked() const noexcept
	{
		return (mFormatFlags & kAudioFormatFlagIsPacked) == kAudioFormatFlagIsPacked;
	}

	/// Returns true if this format is implicitly packed.
	///
	/// A format is implicitly packed when ((mBitsPerChannel / 8) * InterleavedChannelCount()) == mBytesPerFrame
	bool IsImplicitlyPacked() const noexcept
	{
		return ((mBitsPerChannel / 8) * InterleavedChannelCount()) == mBytesPerFrame;
	}

	/// Returns true if this format is linear PCM and the sample bits do not occupy the entire channel.
	bool IsUnpackedPCM() const noexcept
	{
		return IsPCM() && (SampleWordSize() << 3) != mBitsPerChannel;
	}

	/// Returns true if the kAudioFormatFlagIsAlignedHigh flag is set.
	bool IsAlignedHigh() const noexcept
	{
		return (mFormatFlags & kAudioFormatFlagIsAlignedHigh) == kAudioFormatFlagIsAlignedHigh;
	}

	/// Returns true if this format is unpacked linear PCM or if mBitsPerChannel is not a multiple of 8.
	bool IsUnaligned() const noexcept
	{
		return IsUnpackedPCM() || (mBitsPerChannel & 7) != 0;
	}

	/// Returns the number of fractional bits.
	UInt32 FractionalBits() const noexcept
	{
		return (mFormatFlags & kLinearPCMFormatFlagsSampleFractionMask) >> kLinearPCMFormatFlagsSampleFractionShift;
	}

	/// Returns true if this format is integer fixed-point linear PCM.
	bool IsFixedPoint() const noexcept
	{
		return IsInteger() && FractionalBits() > 0;
	}

	/// Returns true if the kAudioFormatFlagIsNonMixable flag is set.
	/// @note This flag is only used when interacting with HAL stream formats.
	bool IsNonMixable() const noexcept
	{
		return (mFormatFlags & kAudioFormatFlagIsNonMixable) == kAudioFormatFlagIsNonMixable;
	}

	/// Returns true if this format is linear PCM and the kAudioFormatFlagIsNonMixable flag is clear.
	/// @note This flag is only used when interacting with HAL stream formats.
	bool IsMixable() const noexcept
	{
		return IsPCM() && (mFormatFlags & kAudioFormatFlagIsNonMixable) == 0;
	}

	/// Returns the sample word size in bytes.
	UInt32 SampleWordSize() const noexcept
	{
		const auto interleavedChannelCount = InterleavedChannelCount();
		if(interleavedChannelCount == 0 || mBytesPerFrame % interleavedChannelCount != 0)
			return 0;
		return mBytesPerFrame / interleavedChannelCount;
	}

	/// Returns the byte size of frameCount audio frames.
	/// @note This is equivalent to frameCount * mBytesPerFrame.
	UInt32 FrameCountToByteSize(UInt32 frameCount) const noexcept
	{
		return frameCount * mBytesPerFrame;
	}

	/// Returns the frame count of byteSize bytes.
	/// @note This is equivalent to byteSize / mBytesPerFrame.
	UInt32 ByteSizeToFrameCount(UInt32 byteSize) const noexcept
	{
		return byteSize / mBytesPerFrame;
	}

	/// Returns the duration of a single packet in seconds.
	double GetPacketDuration() const noexcept
	{
		return (1 / mSampleRate) * mFramesPerPacket;
	}

	// MARK: Format transformation

	/// Sets format to the equivalent non-interleaved format of this.
	/// @note Fails for non-PCM formats.
	/// @return true on success, false otherwise.
	bool GetNonInterleavedEquivalent(CAStreamDescription& format) const noexcept;

	/// Sets format to the equivalent interleaved format of this.
	/// @note Fails for non-PCM formats.
	bool GetInterleavedEquivalent(CAStreamDescription& format) const noexcept;

	/// Sets format to the equivalent standard format of this.
	/// @note Fails for non-PCM formats.
	/// @return true on success, false otherwise.
	bool GetStandardEquivalent(CAStreamDescription& format) const noexcept;

	/// Resets the stream description to the default state.
	void Reset() noexcept
	{
		std::memset(this, 0, sizeof(AudioStreamBasicDescription));
	}

	// MARK: Format Name and Description

	/// Returns the name of this format.
	///
	/// This is the value of kAudioFormatProperty_FormatName.
	/// @note The caller is responsible for releasing the returned string
	CFStringRef _Nullable CopyFormatName() const noexcept CF_RETURNS_RETAINED
	{
		return CopyAudioStreamBasicDescriptionFormatName(*this);
	}

	/// Returns a string representation of this format.
	/// @note The caller is responsible for releasing the returned string.
	CFStringRef _Nullable CopyFormatDescription() const noexcept CF_RETURNS_RETAINED
	{
		return CopyAudioStreamBasicDescriptionFormatDescription(*this);
	}

#ifdef __OBJC__
	/// Returns an AVAudioFormat object initialized with this format and no channel layout.
	operator AVAudioFormat * _Nullable () const noexcept
	{
		return [[AVAudioFormat alloc] initWithStreamDescription:this];
	}

	/// Returns the name of this format.
	///
	/// This is the value of kAudioFormatProperty_FormatName.
	NSString * _Nullable FormatName() const noexcept
	{
		return (__bridge_transfer NSString *)CopyFormatName();
	}

	/// Returns a string representation of this format.
	NSString * _Nullable FormatDescription() const noexcept
	{
		return (__bridge_transfer NSString *)CopyFormatDescription();
	}
#endif /* __OBJC__ */
};

} /* namespace CXXCoreAudio */
