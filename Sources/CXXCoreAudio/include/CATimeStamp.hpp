//
// Copyright © 2021-2025 Stephen F. Booth
// Part of https://github.com/sbooth/CXXCoreAudio
// MIT license
//

#pragma once

#import <CoreAudioTypes/CoreAudioTypes.h>

namespace CoreAudio {

/// A class extending the functionality of an AudioTimeStamp structure.
struct CATimeStamp final : public AudioTimeStamp {
	// MARK: Creation and Destruction

	/// Creates an empty time stamp.
	CATimeStamp() noexcept = default;

	/// Creates a time stamp with the specified sample time.
	/// @param sampleTime The desired sample time.
	explicit CATimeStamp(Float64 sampleTime) noexcept
	: AudioTimeStamp{
		.mSampleTime = sampleTime,
		.mFlags = kAudioTimeStampSampleTimeValid}
	{}

	/// Creates a time stamp with the specified host time.
	/// @param hostTime The desired host time.
	explicit CATimeStamp(UInt64 hostTime) noexcept
	: AudioTimeStamp{
		.mHostTime = hostTime,
		.mFlags = kAudioTimeStampHostTimeValid}
	{}

	/// Creates a time stamp with the specified sample and host times.
	/// @param sampleTime The desired sample time.
	/// @param hostTime The desired host time.
	CATimeStamp(Float64 sampleTime, UInt64 hostTime) noexcept
	: AudioTimeStamp{
		.mSampleTime = sampleTime,
		.mHostTime = hostTime,
		.mFlags = kAudioTimeStampSampleTimeValid | kAudioTimeStampHostTimeValid}
	{}

	/// Creates a time stamp with the specified sample and host times and rate scalar.
	/// @param sampleTime The desired sample time.
	/// @param hostTime The desired host time.
	/// @param rateScalar The desired rate scalar.
	CATimeStamp(Float64 sampleTime, UInt64 hostTime, Float64 rateScalar) noexcept
	: AudioTimeStamp{
		.mSampleTime = sampleTime,
		.mHostTime = hostTime,
		.mRateScalar = rateScalar,
		.mFlags = kAudioTimeStampSampleTimeValid | kAudioTimeStampHostTimeValid | kAudioTimeStampRateScalarValid}
	{}

	/// Creates a time stamp copied from a Core Audio time stamp.
	/// @param other The desired time stamp.
	explicit CATimeStamp(const AudioTimeStamp& other) noexcept
	: AudioTimeStamp{other}
	{}

	/// Copy constructor
	CATimeStamp(const CATimeStamp& other) noexcept = default;

	/// Move constructor
	CATimeStamp(CATimeStamp&& other) noexcept = default;

	/// Assignment operator
	CATimeStamp& operator=(const CATimeStamp& other) noexcept = default;

	/// Assignment operator
	CATimeStamp& operator=(const AudioTimeStamp& other) noexcept
	{
		AudioTimeStamp::operator=(other);
		return *this;
	}

	/// Move assignment operator
	CATimeStamp& operator=(CATimeStamp&& other) noexcept = default;

	/// Destructor
	~CATimeStamp() noexcept = default;

	// MARK: Comparison

	/// Returns true if other is equal to this.
	bool operator==(const AudioTimeStamp& other) const noexcept;

	/// Returns true if other is not equal to this.
	bool operator!=(const AudioTimeStamp& other) const noexcept;

	/// Returns true if other is less than this.
	bool operator<(const AudioTimeStamp& other) const noexcept;

	/// Returns true if other is less than or equal to this.
	bool operator<=(const AudioTimeStamp& other) const noexcept;

	/// Returns true if other is greater than or equal to this.
	bool operator>=(const AudioTimeStamp& other) const noexcept;

	/// Returns true if other is greater than this.
	bool operator>(const AudioTimeStamp& other) const noexcept;

	// MARK: Flags

	/// Returns true if the kAudioTimeStampNothingValid flag is clear.
	explicit operator bool() const noexcept
	{
		return IsValid();
	}

	/// Returns true if the kAudioTimeStampNothingValid flag is clear.
	bool IsValid() const noexcept
	{
		return mFlags != kAudioTimeStampNothingValid;
	}

	/// Returns true if the kAudioTimeStampSampleTimeValid flag is set.
	bool SampleTimeIsValid() const noexcept
	{
		return (mFlags & kAudioTimeStampSampleTimeValid) == kAudioTimeStampSampleTimeValid;
	}

	/// Returns true if the kAudioTimeStampHostTimeValid flag is set.
	bool HostTimeIsValid() const noexcept
	{
		return (mFlags & kAudioTimeStampHostTimeValid) == kAudioTimeStampHostTimeValid;
	}

	/// Returns true if the kAudioTimeStampRateScalarValid flag is set.
	bool RateScalarIsValid() const noexcept
	{
		return (mFlags & kAudioTimeStampRateScalarValid) == kAudioTimeStampRateScalarValid;
	}

	/// Returns true if the kAudioTimeStampWordClockTimeValid flag is set.
	bool WordClockTimeIsValid() const noexcept
	{
		return (mFlags & kAudioTimeStampWordClockTimeValid) == kAudioTimeStampWordClockTimeValid;
	}

	/// Returns true if the kAudioTimeStampSMPTETimeValid flag is set.
	bool SMPTETimeIsValid() const noexcept
	{
		return (mFlags & kAudioTimeStampSMPTETimeValid) == kAudioTimeStampSMPTETimeValid;
	}
};

} /* namespace CoreAudio */
