//
// SPDX-FileCopyrightText: 2025 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXCoreAudio
//

#pragma once

#include <CoreAudioTypes/CoreAudioTypes.h>

#include <algorithm>

namespace CXXCoreAudio {

/// A class extending the functionality of an AudioValueRange structure.
struct CAValueRange final : public AudioValueRange {
  public:
    /// Creates a value range with the minimum and maximum initialized to zero.
    CAValueRange() noexcept = default;

    /// Creates a value range with minimum and maximum values.
    CAValueRange(Float64 minimum, Float64 maximum) noexcept;

    /// Returns true if this value range is valid.
    [[nodiscard]] bool IsValid() const noexcept;

    /// Returns true if this value range contains value.
    [[nodiscard]] bool Contains(Float64 value) const noexcept;

    /// Clamps a value to within the range.
    [[nodiscard]] Float64 Clamp(Float64 value) const noexcept;

    /// Returns true if this value range intersects other.
    [[nodiscard]] bool Intersects(const AudioValueRange& other) const noexcept;

    /// Returns true if this value range contains other.
    [[nodiscard]] bool Contains(const AudioValueRange& other) const noexcept;

    /// Returns true if this value range is equal to another.
    [[nodiscard]] bool operator==(const AudioValueRange& other) const noexcept;

    /// Returns true if this value range is not equal to another.
    [[nodiscard]] bool operator!=(const AudioValueRange& other) const noexcept;
};

// MARK: - Implementation -

inline CAValueRange::CAValueRange(Float64 minimum, Float64 maximum) noexcept
  : AudioValueRange{.mMinimum = minimum, .mMaximum = maximum} {}

inline bool CAValueRange::IsValid() const noexcept {
    return mMaximum >= mMinimum;
}

inline bool CAValueRange::Contains(Float64 value) const noexcept {
    return value >= mMinimum && value <= mMaximum;
}

inline Float64 CAValueRange::Clamp(Float64 value) const noexcept {
    return std::clamp(value, mMinimum, mMaximum);
}

inline bool CAValueRange::Intersects(const AudioValueRange& other) const noexcept {
    return mMinimum <= other.mMaximum && other.mMinimum <= mMaximum;
}

inline bool CAValueRange::Contains(const AudioValueRange& other) const noexcept {
    return mMinimum <= other.mMinimum && other.mMaximum <= mMaximum;
}

inline bool CAValueRange::operator==(const AudioValueRange& other) const noexcept {
    return mMinimum == other.mMinimum && mMaximum == other.mMaximum;
}

inline bool CAValueRange::operator!=(const AudioValueRange& other) const noexcept {
    return !operator==(other);
}

} /* namespace CXXCoreAudio */
