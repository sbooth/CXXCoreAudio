//
// Copyright © 2025 Stephen F. Booth
// Part of https://github.com/sbooth/CXXCoreAudio
// MIT license
//

#pragma once

#import <algorithm>

#import <CoreAudioTypes/CoreAudioTypes.h>

namespace CXXCoreAudio {

/// A class extending the functionality of an AudioValueRange structure.
struct CAValueRange final : public AudioValueRange {
public:
	/// Creates a value range with the minimum and maximum initialized to zero.
	CAValueRange() noexcept = default;

	/// Creates a value range with minimum and maximum values.
	CAValueRange(Float64 minimum, Float64 maximum) noexcept
	: AudioValueRange{minimum, maximum}
	{}

	/// Returns true if this value range is valid.
	bool IsValid() const noexcept
	{ return mMaximum >= mMinimum; }

	/// Returns true if this value range contains value.
	bool Contains(Float64 value) const noexcept
	{ return value >= mMinimum && value <= mMaximum; }

	/// Clamps a value to within the range.
	Float64 Clamp(Float64 value) const noexcept
	{ return std::clamp(value, mMinimum, mMaximum); }

	/// Returns true if this value range intersects other.
	bool Intersects(const AudioValueRange& other) const noexcept
	{ return mMinimum <= other.mMaximum && other.mMinimum <= mMaximum; }

	/// Returns true if this value range contains other.
	bool Contains(const AudioValueRange& other) const noexcept
	{ return mMinimum <= other.mMinimum && other.mMaximum <= mMaximum; }

	/// Returns true if this value range is equal to another.
	bool operator==(const AudioValueRange& other) const noexcept
	{ return mMinimum == other.mMinimum && mMaximum == other.mMaximum; }

	/// Returns true if this value range is not equal to another.
	bool operator!=(const AudioValueRange& other) const noexcept
	{ return !operator==(other); }
};

} /* namespace CXXCoreAudio */
