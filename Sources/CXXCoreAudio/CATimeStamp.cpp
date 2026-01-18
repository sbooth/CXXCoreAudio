//
// SPDX-FileCopyrightText: 2021 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXCoreAudio
//

#import "CATimeStamp.hpp"

// MARK: Comparison

bool CXXCoreAudio::CATimeStamp::operator==(const AudioTimeStamp &other) const noexcept {
    if (SampleTimeIsValid() && (other.mFlags & kAudioTimeStampSampleTimeValid))
        return mSampleTime == other.mSampleTime;
    if (HostTimeIsValid() && (other.mFlags & kAudioTimeStampHostTimeValid))
        return mHostTime == other.mHostTime;
    if (WordClockTimeIsValid() && (other.mFlags & kAudioTimeStampWordClockTimeValid))
        return mWordClockTime == other.mWordClockTime;
    return false;
}

bool CXXCoreAudio::CATimeStamp::operator!=(const AudioTimeStamp &other) const noexcept {
    return !operator==(other);
}

bool CXXCoreAudio::CATimeStamp::operator<(const AudioTimeStamp &other) const noexcept {
    if (SampleTimeIsValid() && (other.mFlags & kAudioTimeStampSampleTimeValid))
        return mSampleTime < other.mSampleTime;
    if (HostTimeIsValid() && (other.mFlags & kAudioTimeStampHostTimeValid))
        return mHostTime < other.mHostTime;
    if (WordClockTimeIsValid() && (other.mFlags & kAudioTimeStampWordClockTimeValid))
        return mWordClockTime < other.mWordClockTime;
    return false;
}

bool CXXCoreAudio::CATimeStamp::operator<=(const AudioTimeStamp &other) const noexcept {
    return operator<(other) || operator==(other);
}

bool CXXCoreAudio::CATimeStamp::operator>(const AudioTimeStamp &other) const noexcept {
    if (SampleTimeIsValid() && (other.mFlags & kAudioTimeStampSampleTimeValid))
        return mSampleTime > other.mSampleTime;
    if (HostTimeIsValid() && (other.mFlags & kAudioTimeStampHostTimeValid))
        return mHostTime > other.mHostTime;
    if (WordClockTimeIsValid() && (other.mFlags & kAudioTimeStampWordClockTimeValid))
        return mWordClockTime > other.mWordClockTime;
    return false;
}

bool CXXCoreAudio::CATimeStamp::operator>=(const AudioTimeStamp &other) const noexcept {
    return operator>(other) || operator==(other);
}
