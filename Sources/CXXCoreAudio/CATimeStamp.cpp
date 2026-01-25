//
// SPDX-FileCopyrightText: 2021 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXCoreAudio
//

#include "CATimeStamp.hpp"

// MARK: Comparison

bool CXXCoreAudio::CATimeStamp::operator==(const AudioTimeStamp& other) const noexcept {
    if (sampleTimeIsValid() && (other.mFlags & kAudioTimeStampSampleTimeValid)) {
        return mSampleTime == other.mSampleTime;
    }
    if (hostTimeIsValid() && (other.mFlags & kAudioTimeStampHostTimeValid)) {
        return mHostTime == other.mHostTime;
    }
    if (wordClockTimeIsValid() && (other.mFlags & kAudioTimeStampWordClockTimeValid)) {
        return mWordClockTime == other.mWordClockTime;
    }
    return false;
}

bool CXXCoreAudio::CATimeStamp::operator!=(const AudioTimeStamp& other) const noexcept {
    return !operator==(other);
}

bool CXXCoreAudio::CATimeStamp::operator<(const AudioTimeStamp& other) const noexcept {
    if (sampleTimeIsValid() && (other.mFlags & kAudioTimeStampSampleTimeValid)) {
        return mSampleTime < other.mSampleTime;
    }
    if (hostTimeIsValid() && (other.mFlags & kAudioTimeStampHostTimeValid)) {
        return mHostTime < other.mHostTime;
    }
    if (wordClockTimeIsValid() && (other.mFlags & kAudioTimeStampWordClockTimeValid)) {
        return mWordClockTime < other.mWordClockTime;
    }
    return false;
}

bool CXXCoreAudio::CATimeStamp::operator<=(const AudioTimeStamp& other) const noexcept {
    return operator<(other) || operator==(other);
}

bool CXXCoreAudio::CATimeStamp::operator>(const AudioTimeStamp& other) const noexcept {
    if (sampleTimeIsValid() && (other.mFlags & kAudioTimeStampSampleTimeValid)) {
        return mSampleTime > other.mSampleTime;
    }
    if (hostTimeIsValid() && (other.mFlags & kAudioTimeStampHostTimeValid)) {
        return mHostTime > other.mHostTime;
    }
    if (wordClockTimeIsValid() && (other.mFlags & kAudioTimeStampWordClockTimeValid)) {
        return mWordClockTime > other.mWordClockTime;
    }
    return false;
}

bool CXXCoreAudio::CATimeStamp::operator>=(const AudioTimeStamp& other) const noexcept {
    return operator>(other) || operator==(other);
}
