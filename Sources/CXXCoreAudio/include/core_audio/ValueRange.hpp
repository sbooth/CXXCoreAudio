//
// SPDX-FileCopyrightText: 2025 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXCoreAudio
//

#pragma once

#include <CoreAudioTypes/CoreAudioTypes.h>

#include <algorithm>

namespace core_audio {

/// A class extending the functionality of an AudioValueRange structure.
struct ValueRange final : public AudioValueRange {
  public:
    /// Creates a value range with the minimum and maximum initialized to zero.
    ValueRange() noexcept = default;

    /// Creates a value range with minimum and maximum values.
    ValueRange(Float64 minimum, Float64 maximum) noexcept;

    /// Returns true if this value range is valid.
    [[nodiscard]] bool isValid() const noexcept;

    /// Returns true if this value range contains value.
    [[nodiscard]] bool contains(Float64 value) const noexcept;

    /// Clamps a value to within the range.
    [[nodiscard]] Float64 clamp(Float64 value) const noexcept;

    /// Returns true if this value range intersects other.
    [[nodiscard]] bool intersects(const AudioValueRange &other) const noexcept;

    /// Returns true if this value range contains other.
    [[nodiscard]] bool contains(const AudioValueRange &other) const noexcept;

    /// Returns true if this value range is equal to another.
    [[nodiscard]] bool operator==(const AudioValueRange &other) const noexcept;

    /// Returns true if this value range is not equal to another.
    [[nodiscard]] bool operator!=(const AudioValueRange &other) const noexcept;
};

// MARK: - Implementation -

inline ValueRange::ValueRange(Float64 minimum, Float64 maximum) noexcept : AudioValueRange{minimum, maximum} {}

inline bool ValueRange::isValid() const noexcept { return mMaximum >= mMinimum; }

inline bool ValueRange::contains(Float64 value) const noexcept { return value >= mMinimum && value <= mMaximum; }

inline Float64 ValueRange::clamp(Float64 value) const noexcept { return std::clamp(value, mMinimum, mMaximum); }

inline bool ValueRange::intersects(const AudioValueRange &other) const noexcept {
    return mMinimum <= other.mMaximum && other.mMinimum <= mMaximum;
}

inline bool ValueRange::contains(const AudioValueRange &other) const noexcept {
    return mMinimum <= other.mMinimum && other.mMaximum <= mMaximum;
}

inline bool ValueRange::operator==(const AudioValueRange &other) const noexcept {
    return mMinimum == other.mMinimum && mMaximum == other.mMaximum;
}

inline bool ValueRange::operator!=(const AudioValueRange &other) const noexcept { return !operator==(other); }

} /* namespace core_audio */
