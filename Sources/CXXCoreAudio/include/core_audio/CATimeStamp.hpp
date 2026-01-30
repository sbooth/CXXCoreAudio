//
// SPDX-FileCopyrightText: 2021 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXCoreAudio
//

#pragma once

#include <CoreAudioTypes/CoreAudioTypes.h>

namespace core_audio {

/// A class extending the functionality of an AudioTimeStamp structure.
struct CATimeStamp final : public AudioTimeStamp {
    // MARK: Construction and Destruction

    /// Creates an empty time stamp.
    CATimeStamp() noexcept = default;

    /// Creates a time stamp with the specified sample time.
    /// @param sampleTime The desired sample time.
    explicit CATimeStamp(Float64 sampleTime) noexcept;

    /// Creates a time stamp with the specified host time.
    /// @param hostTime The desired host time.
    explicit CATimeStamp(UInt64 hostTime) noexcept;

    /// Creates a time stamp with the specified sample and host times.
    /// @param sampleTime The desired sample time.
    /// @param hostTime The desired host time.
    CATimeStamp(Float64 sampleTime, UInt64 hostTime) noexcept;

    /// Creates a time stamp with the specified sample and host times and rate scalar.
    /// @param sampleTime The desired sample time.
    /// @param hostTime The desired host time.
    /// @param rateScalar The desired rate scalar.
    CATimeStamp(Float64 sampleTime, UInt64 hostTime, Float64 rateScalar) noexcept;

    /// Creates a time stamp copied from a Core Audio time stamp.
    /// @param other The desired time stamp.
    explicit CATimeStamp(const AudioTimeStamp &other) noexcept;

    /// Copy constructor
    CATimeStamp(const CATimeStamp &other) noexcept = default;

    /// Move constructor
    CATimeStamp(CATimeStamp &&other) noexcept = default;

    /// Assignment operator
    CATimeStamp &operator=(const CATimeStamp &other) noexcept = default;

    /// Assignment operator
    CATimeStamp &operator=(const AudioTimeStamp &other) noexcept;

    /// Move assignment operator
    CATimeStamp &operator=(CATimeStamp &&other) noexcept = default;

    /// Destructor
    ~CATimeStamp() noexcept = default;

    // MARK: Comparison

    /// Returns true if other is equal to this.
    [[nodiscard]] bool operator==(const AudioTimeStamp &other) const noexcept;

    /// Returns true if other is not equal to this.
    [[nodiscard]] bool operator!=(const AudioTimeStamp &other) const noexcept;

    /// Returns true if other is less than this.
    [[nodiscard]] bool operator<(const AudioTimeStamp &other) const noexcept;

    /// Returns true if other is less than or equal to this.
    [[nodiscard]] bool operator<=(const AudioTimeStamp &other) const noexcept;

    /// Returns true if other is greater than or equal to this.
    [[nodiscard]] bool operator>=(const AudioTimeStamp &other) const noexcept;

    /// Returns true if other is greater than this.
    [[nodiscard]] bool operator>(const AudioTimeStamp &other) const noexcept;

    // MARK: Flags

    /// Returns true if the kAudioTimeStampNothingValid flag is clear.
    [[nodiscard]] explicit operator bool() const noexcept;

    /// Returns true if the kAudioTimeStampNothingValid flag is clear.
    [[nodiscard]] bool isValid() const noexcept;

    /// Returns true if the kAudioTimeStampSampleTimeValid flag is set.
    [[nodiscard]] bool sampleTimeIsValid() const noexcept;

    /// Returns true if the kAudioTimeStampHostTimeValid flag is set.
    [[nodiscard]] bool hostTimeIsValid() const noexcept;

    /// Returns true if the kAudioTimeStampRateScalarValid flag is set.
    [[nodiscard]] bool rateScalarIsValid() const noexcept;

    /// Returns true if the kAudioTimeStampWordClockTimeValid flag is set.
    [[nodiscard]] bool wordClockTimeIsValid() const noexcept;

    /// Returns true if the kAudioTimeStampSMPTETimeValid flag is set.
    [[nodiscard]] bool smpteTimeIsValid() const noexcept;
};

// MARK: - Implementation -

// MARK: Construction and Destruction

inline CATimeStamp::CATimeStamp(Float64 sampleTime) noexcept : AudioTimeStamp{} {
    mSampleTime = sampleTime;
    mFlags = kAudioTimeStampSampleTimeValid;
}

inline CATimeStamp::CATimeStamp(UInt64 hostTime) noexcept : AudioTimeStamp{} {
    mHostTime = hostTime;
    mFlags = kAudioTimeStampHostTimeValid;
}

inline CATimeStamp::CATimeStamp(Float64 sampleTime, UInt64 hostTime) noexcept : AudioTimeStamp{} {
    mSampleTime = sampleTime;
    mHostTime = hostTime;
    mFlags = kAudioTimeStampSampleTimeValid | kAudioTimeStampHostTimeValid;
}

inline CATimeStamp::CATimeStamp(Float64 sampleTime, UInt64 hostTime, Float64 rateScalar) noexcept : AudioTimeStamp{} {
    mSampleTime = sampleTime;
    mHostTime = hostTime;
    mRateScalar = rateScalar;
    mFlags = kAudioTimeStampSampleTimeValid | kAudioTimeStampHostTimeValid | kAudioTimeStampRateScalarValid;
}

inline CATimeStamp::CATimeStamp(const AudioTimeStamp &other) noexcept : AudioTimeStamp(other) {}

inline CATimeStamp &CATimeStamp::operator=(const AudioTimeStamp &other) noexcept {
    AudioTimeStamp::operator=(other);
    return *this;
}

// MARK: Flags

inline CATimeStamp::operator bool() const noexcept { return isValid(); }

inline bool CATimeStamp::isValid() const noexcept { return mFlags != kAudioTimeStampNothingValid; }

inline bool CATimeStamp::sampleTimeIsValid() const noexcept {
    return (mFlags & kAudioTimeStampSampleTimeValid) == kAudioTimeStampSampleTimeValid;
}

inline bool CATimeStamp::hostTimeIsValid() const noexcept {
    return (mFlags & kAudioTimeStampHostTimeValid) == kAudioTimeStampHostTimeValid;
}

inline bool CATimeStamp::rateScalarIsValid() const noexcept {
    return (mFlags & kAudioTimeStampRateScalarValid) == kAudioTimeStampRateScalarValid;
}

inline bool CATimeStamp::wordClockTimeIsValid() const noexcept {
    return (mFlags & kAudioTimeStampWordClockTimeValid) == kAudioTimeStampWordClockTimeValid;
}

inline bool CATimeStamp::smpteTimeIsValid() const noexcept {
    return (mFlags & kAudioTimeStampSMPTETimeValid) == kAudioTimeStampSMPTETimeValid;
}

} /* namespace core_audio */
