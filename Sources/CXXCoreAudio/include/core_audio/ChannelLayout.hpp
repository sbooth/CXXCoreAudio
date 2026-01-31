//
// SPDX-FileCopyrightText: 2013 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXCoreAudio
//

#pragma once

#include <core_audio/malloc_ptr.hpp>

#include <cf/CFRef.hpp>

#include <CoreAudioTypes/CoreAudioTypes.h>
#include <CoreFoundation/CFString.h>

#ifdef __OBJC__
#import <AVFAudio/AVFAudio.h>
#import <Foundation/NSString.h>
#endif /* __OBJC__ */

#include <cstdlib>
#include <utility>
#include <vector>

namespace core_audio {

// MARK: AudioChannelLayout Helper Functions

/// Allocates and returns a new variable-length AudioChannelLayout structure with the specified number of channel
/// descriptions.
/// @note The allocation is performed using std::malloc.
/// @param numberChannelDescriptions The number of channel descriptions that will be stored in the channel layout.
/// @return An AudioChannelLayout struct or null if memory could not be allocated.
[[nodiscard]] malloc_ptr<AudioChannelLayout> allocateAudioChannelLayout(UInt32 numberChannelDescriptions) noexcept;

/// Allocates and returns a copy of a variable-length AudioChannelLayout structure.
/// @note The allocation is performed using std::malloc.
/// @param other The AudioChannelLayout to copy.
/// @return An AudioChannelLayout struct or null if memory could not be allocated.
[[nodiscard]] malloc_ptr<AudioChannelLayout> copyAudioChannelLayout(const AudioChannelLayout *_Nullable other) noexcept;

/// Returns the size required to hold a variable-length AudioChannelLayout structure with the specified number of
/// channel descriptions.
/// @return The required size in bytes.
[[nodiscard]] constexpr size_t audioChannelLayoutSize(UInt32 numberChannelDescriptions) noexcept;

/// Returns the size of a variable-length AudioChannelLayout structure.
/// @return The size of the channel layout in bytes.
[[nodiscard]] size_t audioChannelLayoutSize(const AudioChannelLayout *_Nullable channelLayout) noexcept;

/// Returns the number of channels contained in an audio channel layout.
[[nodiscard]] UInt32 audioChannelLayoutChannelCount(const AudioChannelLayout *_Nullable channelLayout) noexcept;

/// Returns true if two AudioChannelLayout structures are equal.
///
/// This function performs a bitwise comparison based on the number of channel descriptions.
/// @note Two equivalent channel layouts may not be equal.
/// @return true if the AudioChannelLayout structs are equal, false if not.
[[nodiscard]] bool audioChannelLayoutsAreEqual(const AudioChannelLayout *_Nullable lhs,
                                               const AudioChannelLayout *_Nullable rhs) noexcept;

/// Returns true if two AudioChannelLayout structures are equivalent.
///
/// Audio channel layouts are considered equivalent if:
/// 1) Both are null.
/// 2) One is null and the other has a mono or stereo layout tag.
/// 3) kAudioFormatProperty_AreChannelLayoutsEquivalent is true.
/// @note Two equivalent channel layouts may not be equal.
/// @return true if the AudioChannelLayout structs are equivalent, false if not.
[[nodiscard]] bool audioChannelLayoutsAreEquivalent(const AudioChannelLayout *_Nullable lhs,
                                                    const AudioChannelLayout *_Nullable rhs) noexcept;

/// Returns true if two AudioChannelLayout structures are equal.
[[nodiscard]] bool operator==(const AudioChannelLayout &lhs, const AudioChannelLayout &rhs) noexcept;

/// Returns true if two AudioChannelLayout structures are not equal.
[[nodiscard]] bool operator!=(const AudioChannelLayout &lhs, const AudioChannelLayout &rhs) noexcept;

/// Returns the name of the channel layout described by an AudioChannelLayout structure.
///
/// This is the value of kAudioFormatProperty_ChannelLayoutName or kAudioFormatProperty_ChannelLayoutSimpleName.
[[nodiscard]] cf::CFString copyAudioChannelLayoutName(const AudioChannelLayout *_Nullable channelLayout,
                                                      bool simpleName = false) noexcept;

/// Returns a string representation of the channel layout described by an AudioChannelLayout structure.
[[nodiscard]] cf::CFString
copyAudioChannelLayoutDescription(const AudioChannelLayout *_Nullable channelLayout) noexcept;

#ifdef __OBJC__
/// Returns true if two the AVAudioChannelLayout objects are equivalent.
///
/// Audio channel layouts are considered equivalent if:
/// 1) Both are null.
/// 2) One is null and the other has a mono or stereo layout tag.
/// 3) kAudioFormatProperty_AreChannelLayoutsEquivalent is true.
/// @note Two equivalent channel layouts may not be equal.
/// @return true if the AudioChannelLayout structs are equivalent, false if not.
[[nodiscard]] bool avAudioChannelLayoutsAreEquivalent(AVAudioChannelLayout *_Nullable lhs,
                                                      AVAudioChannelLayout *_Nullable rhs) noexcept;

/// Returns the name of the channel layout described by an AudioChannelLayout structure.
///
/// This is the value of kAudioFormatProperty_ChannelLayoutName or kAudioFormatProperty_ChannelLayoutSimpleName.
[[nodiscard]] NSString *_Nullable audioChannelLayoutName(const AudioChannelLayout *_Nullable channelLayout,
                                                         bool simpleName = false) noexcept;

/// Returns a string representation of the channel layout described by an AudioChannelLayout structure.
[[nodiscard]] NSString *_Nullable audioChannelLayoutDescription(
        const AudioChannelLayout *_Nullable channelLayout) noexcept;
#endif /* __OBJC__ */

/// A class simplifying use of the variable-length AudioChannelLayout structure.
class ChannelLayout final {
  public:
    /// Mono layout.
    static const ChannelLayout Mono;

    /// Stereo layout.
    static const ChannelLayout Stereo;

    // MARK: Factory Methods

    /// Creates and returns a channel layout with the specified channel bitmap.
    /// @note The channel bitmap will be converted to a layout tag if possible.
    /// @param channelBitmap The channel bitmap for the channel layout.
    /// @throw std::bad_alloc if memory could not be allocated.
    static ChannelLayout channelLayoutWithBitmap(AudioChannelBitmap channelBitmap);

    /// Creates and returns a channel layout with the specified layout tag.
    /// @param layoutTag The layout tag for the channel layout
    /// @throw std::bad_alloc if memory could not be allocated.
    static ChannelLayout channelLayoutWithTag(AudioChannelLayoutTag layoutTag);

    /// Creates and returns a channel layout with the specified channel labels.
    /// @note The channel labels will be converted to a layout tag if possible.
    /// @param channelLabels The channel labels for the channel layout.
    /// @throw std::bad_alloc if memory could not be allocated.
    static ChannelLayout channelLayoutWithChannelLabels(std::vector<AudioChannelLabel> channelLabels);

    // MARK: Construction and Destruction

    /// Creates an empty channel layout.
    ChannelLayout() noexcept = default;

    /// Creates a channel layout with the specified layout tag.
    /// @param layoutTag The layout tag for the channel layout.
    /// @throw std::bad_alloc if memory could not be allocated.
    explicit ChannelLayout(AudioChannelLayoutTag layoutTag);

    /// Creates a channel layout.
    /// @note The channel labels will be converted to a layout tag if possible.
    /// @param channelLabels The channel labels for the channel layout.
    /// @throw std::bad_alloc if memory could not be allocated.
    explicit ChannelLayout(std::vector<AudioChannelLabel> channelLabels);

    /// Creates a copy of a channel layout.
    /// @throw std::bad_alloc if memory could not be allocated.
    ChannelLayout(const ChannelLayout &other);

    /// Creates a channel layout with a copy of an AudioChannelLayout.
    /// @throw std::bad_alloc if memory could not be allocated.
    ChannelLayout(const AudioChannelLayout *_Nullable other);

    /// Replaces the channel layout with a copy of a channel layout.
    /// @throw std::bad_alloc if memory could not be allocated.
    ChannelLayout &operator=(const ChannelLayout &other);

    /// Replaces the channel layout with a copy of an AudioChannelLayout.
    /// @throw std::bad_alloc if memory could not be allocated.
    ChannelLayout &operator=(const AudioChannelLayout *_Nullable other);

    /// Creates a channel layout by moving the contents of another.
    ChannelLayout(ChannelLayout &&other) noexcept;

    /// Replaces the channel layout with the moved contents of another.
    ChannelLayout &operator=(ChannelLayout &&other) noexcept;

    /// Destroys the channel layout and releases all associated resources.
    ~ChannelLayout() noexcept;

    // MARK: Comparison

    /// Returns true if the channel layout is equal to an AudioChannelLayout.
    ///
    /// This function performs a bitwise comparison based on the number of channel descriptions.
    /// @note Two equivalent channel layouts may not be equal.
    [[nodiscard]] bool isEqual(const AudioChannelLayout *_Nullable other) const noexcept;

    /// Returns true if the channel layout is equal to another channel layout.
    ///
    /// This function performs a bitwise comparison based on the number of channel descriptions.
    /// @note Two equivalent channel layouts may not be equal.
    [[nodiscard]] bool isEqual(const ChannelLayout &other) const noexcept;

    /// Returns true if the channel layout is equal to an AudioChannelLayout.
    [[nodiscard]] bool operator==(const AudioChannelLayout *_Nullable other) const noexcept;

    /// Returns true if the channel layout is not equal to an AudioChannelLayout.
    [[nodiscard]] bool operator!=(const AudioChannelLayout *_Nullable other) const noexcept;

    /// Returns true if the channel layout is equal to another.
    [[nodiscard]] bool operator==(const ChannelLayout &other) const noexcept;

    /// Returns true if the channel layout is not equal to another.
    [[nodiscard]] bool operator!=(const ChannelLayout &other) const noexcept;

    // MARK: Equivalence

    /// Returns true if the channel layout is equivalent to an AudioChannelLayout.
    ///
    /// Channel layouts are considered equivalent if:
    /// 1) Both are empty.
    /// 2) One is empty and the other has a mono or stereo layout tag.
    /// 3) kAudioFormatProperty_AreChannelLayoutsEquivalent is true.
    /// @note Two equivalent channel layouts may not be equal.
    [[nodiscard]] bool isEquivalent(const AudioChannelLayout *_Nullable other) const noexcept;

    /// Returns true if the channel layout is equivalent to another channel layout.
    ///
    /// Channel layouts are considered equivalent if:
    /// 1) Both are empty.
    /// 2) One is empty and the other has a mono or stereo layout tag.
    /// 3) kAudioFormatProperty_AreChannelLayoutsEquivalent is true.
    /// @note Two equivalent channel layouts may not be equal.
    [[nodiscard]] bool isEquivalent(const ChannelLayout &other) const noexcept;

    // MARK: Functionality

    /// Returns the number of channels contained in this channel layout.
    [[nodiscard]] UInt32 channelCount() const noexcept;

    /// Creates a channel map for remapping audio from this channel layout.
    /// @param outputLayout The output channel layout
    /// @param channelMap A std::vector to receive the channel map on success
    /// @return true on success, false otherwise
    /// @throw std::bad_alloc if memory could not be allocated.
    bool mapToLayout(const ChannelLayout &outputLayout, std::vector<SInt32> &channelMap) const;

    // MARK: AudioChannelLayout access

    /// Returns the size in bytes of the managed AudioChannelLayout struct.
    [[nodiscard]] size_t size() const noexcept;

    /// A channel layout is empty when the managed AudioChannelLayout struct is null.
    [[nodiscard]] explicit operator bool() const noexcept;

    /// Returns a const pointer to the managed AudioChannelLayout struct.
    [[nodiscard]] const AudioChannelLayout *_Nullable operator->() const noexcept;

    /// Returns a const pointer to the managed AudioChannelLayout struct.
    [[nodiscard]] operator const AudioChannelLayout *const _Nullable() const noexcept;

    // MARK: Channel Layout Name and Description

    /// Returns the name of this channel layout.
    ///
    /// This is the value of kAudioFormatProperty_ChannelLayoutName or kAudioFormatProperty_ChannelLayoutSimpleName.
    [[nodiscard]] cf::CFString copyLayoutName(bool simpleName = false) const noexcept;

    /// Returns a string representation of this channel layout
    ///
    /// This is the value of kAudioFormatProperty_ChannelLayoutName or kAudioFormatProperty_ChannelLayoutSimpleName.
    [[nodiscard]] cf::CFString copyLayoutDescription() const noexcept;

#ifdef __OBJC__
    /// Returns an AVAudioChannelLayout object initialized with the managed AudioChannelLayout struct.
    [[nodiscard]] operator AVAudioChannelLayout *_Nullable() const noexcept;

    /// Returns the name of this channel layout.
    [[nodiscard]] NSString *_Nullable layoutName(bool simpleName = false) const noexcept;

    /// Returns a string representation of this channel layout.
    [[nodiscard]] NSString *_Nullable layoutDescription() const noexcept;
#endif /* __OBJC__ */

    /// Returns the managed AudioChannelLayout struct.
    [[nodiscard]] const AudioChannelLayout *_Nullable get() const noexcept;

    /// Replaces the managed AudioChannelLayout struct with another AudioChannelLayout struct.
    /// @note The object assumes responsibility for deallocating the passed AudioChannelLayout struct using std::free.
    void reset(AudioChannelLayout *_Nullable channelLayout = nullptr) noexcept;

    /// Swaps the managed AudioChannelLayout struct with the managed AudioChannelLayout struct from another audio
    /// channel layout.
    void swap(ChannelLayout &other) noexcept;

    /// Releases ownership of the managed AudioChannelLayout struct and returns it.
    /// @note The caller assumes responsibility for deallocating the returned AudioChannelLayout struct using std::free.
    [[nodiscard]] AudioChannelLayout *_Nullable release() noexcept;

  private:
    /// The managed AudioChannelLayout structure.
    AudioChannelLayout *_Nullable channelLayout_{nullptr};
};

// MARK: - Implementation -

constexpr size_t audioChannelLayoutSize(UInt32 numberChannelDescriptions) noexcept {
    return offsetof(AudioChannelLayout, mChannelDescriptions) +
           (numberChannelDescriptions * sizeof(AudioChannelDescription));
}

inline size_t audioChannelLayoutSize(const AudioChannelLayout *_Nullable channelLayout) noexcept {
    if (channelLayout == nullptr) {
        return 0;
    }
    return audioChannelLayoutSize(channelLayout->mNumberChannelDescriptions);
}

inline bool operator==(const AudioChannelLayout &lhs, const AudioChannelLayout &rhs) noexcept {
    return audioChannelLayoutsAreEqual(&lhs, &rhs);
}

inline bool operator!=(const AudioChannelLayout &lhs, const AudioChannelLayout &rhs) noexcept {
    return !operator==(lhs, rhs);
}

#ifdef __OBJC__
inline bool avAudioChannelLayoutsAreEquivalent(AVAudioChannelLayout *_Nullable lhs,
                                               AVAudioChannelLayout *_Nullable rhs) noexcept {
    return audioChannelLayoutsAreEquivalent(lhs.layout, rhs.layout);
}

inline NSString *_Nullable audioChannelLayoutName(const AudioChannelLayout *_Nullable channelLayout,
                                                  bool simpleName) noexcept {
    auto layoutName = copyAudioChannelLayoutName(channelLayout, simpleName);
    return (__bridge_transfer NSString *)layoutName.leak();
}

inline NSString *_Nullable audioChannelLayoutDescription(const AudioChannelLayout *_Nullable channelLayout) noexcept {
    auto layoutDescription = copyAudioChannelLayoutDescription(channelLayout);
    return (__bridge_transfer NSString *)layoutDescription.leak();
}
#endif /* __OBJC__ */

// MARK: Comparison

inline bool ChannelLayout::isEqual(const AudioChannelLayout *_Nullable other) const noexcept {
    return audioChannelLayoutsAreEqual(channelLayout_, other);
}

inline bool ChannelLayout::isEqual(const ChannelLayout &other) const noexcept { return isEqual(other.channelLayout_); }

inline bool ChannelLayout::operator==(const AudioChannelLayout *_Nullable other) const noexcept {
    return isEqual(other);
}

inline bool ChannelLayout::operator!=(const AudioChannelLayout *_Nullable other) const noexcept {
    return !operator==(other);
}

inline bool ChannelLayout::operator==(const ChannelLayout &other) const noexcept {
    return operator==(other.channelLayout_);
}

inline bool ChannelLayout::operator!=(const ChannelLayout &other) const noexcept {
    return !operator==(other.channelLayout_);
}

// MARK: Equivalence

inline bool ChannelLayout::isEquivalent(const AudioChannelLayout *_Nullable other) const noexcept {
    return audioChannelLayoutsAreEquivalent(channelLayout_, other);
}

inline bool ChannelLayout::isEquivalent(const ChannelLayout &other) const noexcept {
    return isEquivalent(other.channelLayout_);
}

// MARK: Functionality

inline UInt32 ChannelLayout::channelCount() const noexcept { return audioChannelLayoutChannelCount(channelLayout_); }

// MARK: AudioChannelLayout access

inline size_t ChannelLayout::size() const noexcept { return audioChannelLayoutSize(channelLayout_); }

inline ChannelLayout::operator bool() const noexcept { return channelLayout_ != nullptr; }

inline const AudioChannelLayout *_Nullable ChannelLayout::operator->() const noexcept { return channelLayout_; }

inline ChannelLayout::operator const AudioChannelLayout *const _Nullable() const noexcept { return channelLayout_; }

// MARK: Channel Layout Name and Description

inline cf::CFString ChannelLayout::copyLayoutName(bool simpleName) const noexcept {
    return copyAudioChannelLayoutName(channelLayout_, simpleName);
}

inline cf::CFString ChannelLayout::copyLayoutDescription() const noexcept {
    return copyAudioChannelLayoutDescription(channelLayout_);
}

#ifdef __OBJC__
inline ChannelLayout::operator AVAudioChannelLayout *_Nullable() const noexcept {
    return [[AVAudioChannelLayout alloc] initWithLayout:channelLayout_];
}

inline NSString *_Nullable ChannelLayout::layoutName(bool simpleName) const noexcept {
    return audioChannelLayoutName(channelLayout_, simpleName);
}

inline NSString *_Nullable ChannelLayout::layoutDescription() const noexcept {
    return audioChannelLayoutDescription(channelLayout_);
}
#endif /* __OBJC__ */

inline const AudioChannelLayout *_Nullable ChannelLayout::get() const noexcept { return channelLayout_; }

inline void ChannelLayout::reset(AudioChannelLayout *_Nullable channelLayout) noexcept {
    std::free(std::exchange(channelLayout_, channelLayout));
}

inline void ChannelLayout::swap(ChannelLayout &other) noexcept { std::swap(channelLayout_, other.channelLayout_); }

inline AudioChannelLayout *_Nullable ChannelLayout::release() noexcept {
    return std::exchange(channelLayout_, nullptr);
}

} /* namespace core_audio */
