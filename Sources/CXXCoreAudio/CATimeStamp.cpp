//
// SPDX-FileCopyrightText: 2021 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXCoreAudio
//

#include "core_audio/CATimeStamp.hpp"

// MARK: Comparison

bool core_audio::CATimeStamp::operator==(const AudioTimeStamp &other) const noexcept {
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

bool core_audio::CATimeStamp::operator!=(const AudioTimeStamp &other) const noexcept { return !operator==(other); }

bool core_audio::CATimeStamp::operator<(const AudioTimeStamp &other) const noexcept {
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

bool core_audio::CATimeStamp::operator<=(const AudioTimeStamp &other) const noexcept {
    return operator<(other) || operator==(other);
}

bool core_audio::CATimeStamp::operator>(const AudioTimeStamp &other) const noexcept {
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

bool core_audio::CATimeStamp::operator>=(const AudioTimeStamp &other) const noexcept {
    return operator>(other) || operator==(other);
}
