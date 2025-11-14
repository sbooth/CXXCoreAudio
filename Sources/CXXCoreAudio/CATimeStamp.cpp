//
// Copyright © 2021-2025 Stephen F. Booth
// Part of https://github.com/sbooth/CXXCoreAudio
// MIT license
//

#import "CATimeStamp.hpp"

// MARK: Comparison

bool CXXCoreAudio::CATimeStamp::operator==(const AudioTimeStamp& other) const noexcept
{
	if(SampleTimeIsValid() && (other.mFlags & kAudioTimeStampSampleTimeValid))
		return mSampleTime == other.mSampleTime;
	if(HostTimeIsValid() && (other.mFlags & kAudioTimeStampHostTimeValid))
		return mHostTime == other.mHostTime;
	if(WordClockTimeIsValid() && (other.mFlags & kAudioTimeStampWordClockTimeValid))
		return mWordClockTime == other.mWordClockTime;
	return false;
}

bool CXXCoreAudio::CATimeStamp::operator!=(const AudioTimeStamp& other) const noexcept
{
	return !operator==(other);
}

bool CXXCoreAudio::CATimeStamp::operator<(const AudioTimeStamp& other) const noexcept
{
	if(SampleTimeIsValid() && (other.mFlags & kAudioTimeStampSampleTimeValid))
		return mSampleTime < other.mSampleTime;
	if(HostTimeIsValid() && (other.mFlags & kAudioTimeStampHostTimeValid))
		return mHostTime < other.mHostTime;
	if(WordClockTimeIsValid() && (other.mFlags & kAudioTimeStampWordClockTimeValid))
		return mWordClockTime < other.mWordClockTime;
	return false;
}

bool CXXCoreAudio::CATimeStamp::operator<=(const AudioTimeStamp& other) const noexcept
{
	return operator<(other) || operator==(other);
}

bool CXXCoreAudio::CATimeStamp::operator>(const AudioTimeStamp& other) const noexcept
{
	if(SampleTimeIsValid() && (other.mFlags & kAudioTimeStampSampleTimeValid))
		return mSampleTime > other.mSampleTime;
	if(HostTimeIsValid() && (other.mFlags & kAudioTimeStampHostTimeValid))
		return mHostTime > other.mHostTime;
	if(WordClockTimeIsValid() && (other.mFlags & kAudioTimeStampWordClockTimeValid))
		return mWordClockTime > other.mWordClockTime;
	return false;
}

bool CXXCoreAudio::CATimeStamp::operator>=(const AudioTimeStamp& other) const noexcept
{
	return operator>(other) || operator==(other);
}
