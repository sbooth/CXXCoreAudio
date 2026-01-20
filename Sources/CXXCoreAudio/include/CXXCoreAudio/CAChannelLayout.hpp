//
// SPDX-FileCopyrightText: 2013 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXCoreAudio
//

#pragma once

#import <CXXCoreAudio/malloc_ptr.hpp>

#import <cxx_cf/CFRef.hpp>

#import <CoreAudioTypes/CoreAudioTypes.h>
#import <CoreFoundation/CFString.h>
#ifdef __OBJC__
#import <AVFAudio/AVFAudio.h>
#import <Foundation/NSString.h>
#endif /* __OBJC__ */

#import <cstdlib>
#import <utility>
#import <vector>

namespace CXXCoreAudio {

// MARK: AudioChannelLayout Helper Functions

/// Allocates and returns a new variable-length AudioChannelLayout structure with the specified number of channel
/// descriptions.
/// @note The allocation is performed using std::malloc.
/// @param numberChannelDescriptions The number of channel descriptions that will be stored in the channel layout.
/// @return An AudioChannelLayout struct or null if memory could not be allocated.
[[nodiscard]] malloc_ptr<AudioChannelLayout> AllocateAudioChannelLayout(UInt32 numberChannelDescriptions) noexcept;

/// Allocates and returns a copy of a variable-length AudioChannelLayout structure.
/// @note The allocation is performed using std::malloc.
/// @param other The AudioChannelLayout to copy.
/// @return An AudioChannelLayout struct or null if memory could not be allocated.
[[nodiscard]] malloc_ptr<AudioChannelLayout> CopyAudioChannelLayout(const AudioChannelLayout *_Nullable other) noexcept;

/// Returns the size required to hold a variable-length AudioChannelLayout structure with the specified number of
/// channel descriptions.
/// @return The required size in bytes.
[[nodiscard]] constexpr size_t AudioChannelLayoutSize(UInt32 numberChannelDescriptions) noexcept;

/// Returns the size of a variable-length AudioChannelLayout structure.
/// @return The size of the channel layout in bytes.
[[nodiscard]] size_t AudioChannelLayoutSize(const AudioChannelLayout *_Nullable channelLayout) noexcept;

/// Returns the number of channels contained in an audio channel layout.
[[nodiscard]] UInt32 AudioChannelLayoutChannelCount(const AudioChannelLayout *_Nullable channelLayout) noexcept;

/// Returns true if two AudioChannelLayout structures are equal.
///
/// This function performs a bitwise comparison based on the number of channel descriptions.
/// @note Two equivalent channel layouts may not be equal.
/// @return true if the AudioChannelLayout structs are equal, false if not.
[[nodiscard]] bool AudioChannelLayoutsAreEqual(const AudioChannelLayout *_Nullable lhs,
                                               const AudioChannelLayout *_Nullable rhs) noexcept;

/// Returns true if two AudioChannelLayout structures are equivalent.
///
/// Audio channel layouts are considered equivalent if:
/// 1) Both are null.
/// 2) One is null and the other has a mono or stereo layout tag.
/// 3) kAudioFormatProperty_AreChannelLayoutsEquivalent is true.
/// @note Two equivalent channel layouts may not be equal.
/// @return true if the AudioChannelLayout structs are equivalent, false if not.
[[nodiscard]] bool AudioChannelLayoutsAreEquivalent(const AudioChannelLayout *_Nullable lhs,
                                                    const AudioChannelLayout *_Nullable rhs) noexcept;

/// Returns true if two AudioChannelLayout structures are equal.
[[nodiscard]] bool operator==(const AudioChannelLayout& lhs, const AudioChannelLayout& rhs) noexcept;

/// Returns true if two AudioChannelLayout structures are not equal.
[[nodiscard]] bool operator!=(const AudioChannelLayout& lhs, const AudioChannelLayout& rhs) noexcept;

/// Returns the name of the channel layout described by an AudioChannelLayout structure.
///
/// This is the value of kAudioFormatProperty_ChannelLayoutName or kAudioFormatProperty_ChannelLayoutSimpleName.
[[nodiscard]] cxx_cf::CFString CopyAudioChannelLayoutName(const AudioChannelLayout *_Nullable channelLayout,
                                                          bool simpleName = false) noexcept;

/// Returns a string representation of the channel layout described by an AudioChannelLayout structure.
[[nodiscard]] cxx_cf::CFString
CopyAudioChannelLayoutDescription(const AudioChannelLayout *_Nullable channelLayout) noexcept;

#ifdef __OBJC__
/// Returns true if two the AVAudioChannelLayout objects are equivalent.
///
/// Audio channel layouts are considered equivalent if:
/// 1) Both are null.
/// 2) One is null and the other has a mono or stereo layout tag.
/// 3) kAudioFormatProperty_AreChannelLayoutsEquivalent is true.
/// @note Two equivalent channel layouts may not be equal.
/// @return true if the AudioChannelLayout structs are equivalent, false if not.
[[nodiscard]] bool AVAudioChannelLayoutsAreEquivalent(AVAudioChannelLayout *_Nullable lhs,
                                                      AVAudioChannelLayout *_Nullable rhs) noexcept;

/// Returns the name of the channel layout described by an AudioChannelLayout structure.
///
/// This is the value of kAudioFormatProperty_ChannelLayoutName or kAudioFormatProperty_ChannelLayoutSimpleName.
[[nodiscard]] NSString *_Nullable AudioChannelLayoutName(const AudioChannelLayout *_Nullable channelLayout,
                                                         bool simpleName = false) noexcept;

/// Returns a string representation of the channel layout described by an AudioChannelLayout structure.
[[nodiscard]] NSString *_Nullable AudioChannelLayoutDescription(
      const AudioChannelLayout *_Nullable channelLayout) noexcept;
#endif /* __OBJC__ */

/// A class simplifying use of the variable-length AudioChannelLayout structure.
class CAChannelLayout final {
  public:
    /// Mono layout.
    static const CAChannelLayout Mono;

    /// Stereo layout.
    static const CAChannelLayout Stereo;

    // MARK: Factory Methods

    /// Creates and returns a channel layout with the specified channel bitmap.
    /// @note The channel bitmap will be converted to a layout tag if possible.
    /// @param channelBitmap The channel bitmap for the channel layout.
    /// @throw std::bad_alloc if memory could not be allocated.
    static CAChannelLayout ChannelLayoutWithBitmap(AudioChannelBitmap channelBitmap);

    /// Creates and returns a channel layout with the specified layout tag.
    /// @param layoutTag The layout tag for the channel layout
    /// @throw std::bad_alloc if memory could not be allocated.
    static CAChannelLayout ChannelLayoutWithTag(AudioChannelLayoutTag layoutTag);

    /// Creates and returns a channel layout with the specified channel labels.
    /// @note The channel labels will be converted to a layout tag if possible.
    /// @param channelLabels The channel labels for the channel layout.
    /// @throw std::bad_alloc if memory could not be allocated.
    static CAChannelLayout ChannelLayoutWithChannelLabels(std::vector<AudioChannelLabel> channelLabels);

    // MARK: Creation and Destruction

    /// Creates an empty channel layout.
    CAChannelLayout() noexcept = default;

    /// Creates a channel layout with the specified layout tag.
    /// @param layoutTag The layout tag for the channel layout.
    /// @throw std::bad_alloc if memory could not be allocated.
    explicit CAChannelLayout(AudioChannelLayoutTag layoutTag);

    /// Creates a channel layout.
    /// @note The channel labels will be converted to a layout tag if possible.
    /// @param channelLabels The channel labels for the channel layout.
    /// @throw std::bad_alloc if memory could not be allocated.
    explicit CAChannelLayout(std::vector<AudioChannelLabel> channelLabels);

    /// Creates a copy of a channel layout.
    /// @throw std::bad_alloc if memory could not be allocated.
    CAChannelLayout(const CAChannelLayout& other);

    /// Creates a channel layout with a copy of an AudioChannelLayout.
    /// @throw std::bad_alloc if memory could not be allocated.
    CAChannelLayout(const AudioChannelLayout *_Nullable other);

    /// Replaces the channel layout with a copy of a channel layout.
    /// @throw std::bad_alloc if memory could not be allocated.
    CAChannelLayout& operator=(const CAChannelLayout& other);

    /// Replaces the channel layout with a copy of an AudioChannelLayout.
    /// @throw std::bad_alloc if memory could not be allocated.
    CAChannelLayout& operator=(const AudioChannelLayout *_Nullable other);

    /// Creates a channel layout by moving the contents of another.
    CAChannelLayout(CAChannelLayout&& other) noexcept;

    /// Replaces the channel layout with the moved contents of another.
    CAChannelLayout& operator=(CAChannelLayout&& other) noexcept;

    /// Destroys the channel layout and releases all associated resources.
    ~CAChannelLayout() noexcept;

    // MARK: Comparison

    /// Returns true if the channel layout is equal to an AudioChannelLayout.
    ///
    /// This function performs a bitwise comparison based on the number of channel descriptions.
    /// @note Two equivalent channel layouts may not be equal.
    [[nodiscard]] bool IsEqual(const AudioChannelLayout *_Nullable other) const noexcept;

    /// Returns true if the channel layout is equal to another channel layout.
    ///
    /// This function performs a bitwise comparison based on the number of channel descriptions.
    /// @note Two equivalent channel layouts may not be equal.
    [[nodiscard]] bool IsEqual(const CAChannelLayout& other) const noexcept;

    /// Returns true if the channel layout is equal to an AudioChannelLayout.
    [[nodiscard]] bool operator==(const AudioChannelLayout *_Nullable other) const noexcept;

    /// Returns true if the channel layout is not equal to an AudioChannelLayout.
    [[nodiscard]] bool operator!=(const AudioChannelLayout *_Nullable other) const noexcept;

    /// Returns true if the channel layout is equal to another.
    [[nodiscard]] bool operator==(const CAChannelLayout& other) const noexcept;

    /// Returns true if the channel layout is not equal to another.
    [[nodiscard]] bool operator!=(const CAChannelLayout& other) const noexcept;

    // MARK: Equivalence

    /// Returns true if the channel layout is equivalent to an AudioChannelLayout.
    ///
    /// Channel layouts are considered equivalent if:
    /// 1) Both are empty.
    /// 2) One is empty and the other has a mono or stereo layout tag.
    /// 3) kAudioFormatProperty_AreChannelLayoutsEquivalent is true.
    /// @note Two equivalent channel layouts may not be equal.
    [[nodiscard]] bool IsEquivalent(const AudioChannelLayout *_Nullable other) const noexcept;

    /// Returns true if the channel layout is equivalent to another channel layout.
    ///
    /// Channel layouts are considered equivalent if:
    /// 1) Both are empty.
    /// 2) One is empty and the other has a mono or stereo layout tag.
    /// 3) kAudioFormatProperty_AreChannelLayoutsEquivalent is true.
    /// @note Two equivalent channel layouts may not be equal.
    [[nodiscard]] bool IsEquivalent(const CAChannelLayout& other) const noexcept;

    // MARK: Functionality

    /// Returns the number of channels contained in this channel layout.
    [[nodiscard]] UInt32 ChannelCount() const noexcept;

    /// Creates a channel map for remapping audio from this channel layout.
    /// @param outputLayout The output channel layout
    /// @param channelMap A std::vector to receive the channel map on success
    /// @return true on success, false otherwise
    /// @throw std::bad_alloc if memory could not be allocated.
    bool MapToLayout(const CAChannelLayout& outputLayout, std::vector<SInt32>& channelMap) const;

    // MARK: AudioChannelLayout access

    /// Returns the size in bytes of the managed AudioChannelLayout struct.
    [[nodiscard]] size_t Size() const noexcept;

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
    [[nodiscard]] cxx_cf::CFString CopyLayoutName(bool simpleName = false) const noexcept;

    /// Returns a string representation of this channel layout
    ///
    /// This is the value of kAudioFormatProperty_ChannelLayoutName or kAudioFormatProperty_ChannelLayoutSimpleName.
    [[nodiscard]] cxx_cf::CFString CopyLayoutDescription() const noexcept;

#ifdef __OBJC__
    /// Returns an AVAudioChannelLayout object initialized with the managed AudioChannelLayout struct.
    [[nodiscard]] operator AVAudioChannelLayout *_Nullable() const noexcept;

    /// Returns the name of this channel layout.
    [[nodiscard]] NSString *_Nullable LayoutName(bool simpleName = false) const noexcept;

    /// Returns a string representation of this channel layout.
    [[nodiscard]] NSString *_Nullable LayoutDescription() const noexcept;
#endif /* __OBJC__ */

    /// Returns the managed AudioChannelLayout struct.
    [[nodiscard]] const AudioChannelLayout *_Nullable get() const noexcept;

    /// Replaces the managed AudioChannelLayout struct with another AudioChannelLayout struct.
    /// @note The object assumes responsibility for deallocating the passed AudioChannelLayout struct using std::free.
    void reset(AudioChannelLayout *_Nullable channelLayout = nullptr) noexcept;

    /// Swaps the managed AudioChannelLayout struct with the managed AudioChannelLayout struct from another audio
    /// channel layout.
    void swap(CAChannelLayout& other) noexcept;

    /// Releases ownership of the managed AudioChannelLayout struct and returns it.
    /// @note The caller assumes responsibility for deallocating the returned AudioChannelLayout struct using std::free.
    [[nodiscard]] AudioChannelLayout *_Nullable release() noexcept;

  private:
    /// The managed AudioChannelLayout structure.
    AudioChannelLayout *_Nullable channelLayout_{nullptr};
};

// MARK: - Implementation -

constexpr size_t AudioChannelLayoutSize(UInt32 numberChannelDescriptions) noexcept {
    return offsetof(AudioChannelLayout, mChannelDescriptions) +
           (numberChannelDescriptions * sizeof(AudioChannelDescription));
}

inline size_t AudioChannelLayoutSize(const AudioChannelLayout *_Nullable channelLayout) noexcept {
    if (!channelLayout)
        return 0;
    return AudioChannelLayoutSize(channelLayout->mNumberChannelDescriptions);
}

inline bool operator==(const AudioChannelLayout& lhs, const AudioChannelLayout& rhs) noexcept {
    return AudioChannelLayoutsAreEqual(&lhs, &rhs);
}

inline bool operator!=(const AudioChannelLayout& lhs, const AudioChannelLayout& rhs) noexcept {
    return !operator==(lhs, rhs);
}

#ifdef __OBJC__
inline bool AVAudioChannelLayoutsAreEquivalent(AVAudioChannelLayout *_Nullable lhs,
                                               AVAudioChannelLayout *_Nullable rhs) noexcept {
    return AudioChannelLayoutsAreEquivalent(lhs.layout, rhs.layout);
}

inline NSString *_Nullable AudioChannelLayoutName(const AudioChannelLayout *_Nullable channelLayout,
                                                  bool simpleName) noexcept {
    auto layoutName = CopyAudioChannelLayoutName(channelLayout, simpleName);
    return (__bridge_transfer NSString *)layoutName.release();
}

/// Returns a string representation of the channel layout described by an AudioChannelLayout structure.
inline NSString *_Nullable AudioChannelLayoutDescription(const AudioChannelLayout *_Nullable channelLayout) noexcept {
    auto layoutDescription = CopyAudioChannelLayoutDescription(channelLayout);
    return (__bridge_transfer NSString *)layoutDescription.release();
}
#endif /* __OBJC__ */

// MARK: Comparison

inline bool CAChannelLayout::IsEqual(const AudioChannelLayout *_Nullable other) const noexcept {
    return AudioChannelLayoutsAreEqual(channelLayout_, other);
}

inline bool CAChannelLayout::IsEqual(const CAChannelLayout& other) const noexcept {
    return IsEqual(other.channelLayout_);
}

inline bool CAChannelLayout::operator==(const AudioChannelLayout *_Nullable other) const noexcept {
    return IsEqual(other);
}

inline bool CAChannelLayout::operator!=(const AudioChannelLayout *_Nullable other) const noexcept {
    return !operator==(other);
}

inline bool CAChannelLayout::operator==(const CAChannelLayout& other) const noexcept {
    return operator==(other.channelLayout_);
}

inline bool CAChannelLayout::operator!=(const CAChannelLayout& other) const noexcept {
    return !operator==(other.channelLayout_);
}

// MARK: Equivalence

inline bool CAChannelLayout::IsEquivalent(const AudioChannelLayout *_Nullable other) const noexcept {
    return AudioChannelLayoutsAreEquivalent(channelLayout_, other);
}

inline bool CAChannelLayout::IsEquivalent(const CAChannelLayout& other) const noexcept {
    return IsEquivalent(other.channelLayout_);
}

// MARK: Functionality

inline UInt32 CAChannelLayout::ChannelCount() const noexcept {
    return AudioChannelLayoutChannelCount(channelLayout_);
}

// MARK: AudioChannelLayout access

/// Returns the size in bytes of the managed AudioChannelLayout struct.
inline size_t CAChannelLayout::Size() const noexcept {
    return AudioChannelLayoutSize(channelLayout_);
}

inline CAChannelLayout::operator bool() const noexcept {
    return channelLayout_ != nullptr;
}

inline const AudioChannelLayout *_Nullable CAChannelLayout::operator->() const noexcept {
    return channelLayout_;
}

inline CAChannelLayout::operator const AudioChannelLayout *const _Nullable() const noexcept {
    return channelLayout_;
}

// MARK: Channel Layout Name and Description

inline cxx_cf::CFString CAChannelLayout::CopyLayoutName(bool simpleName) const noexcept {
    return CopyAudioChannelLayoutName(channelLayout_, simpleName);
}

inline cxx_cf::CFString CAChannelLayout::CopyLayoutDescription() const noexcept {
    return CopyAudioChannelLayoutDescription(channelLayout_);
}

#ifdef __OBJC__
inline CAChannelLayout::operator AVAudioChannelLayout *_Nullable() const noexcept {
    return [[AVAudioChannelLayout alloc] initWithLayout:channelLayout_];
}

inline NSString *_Nullable CAChannelLayout::LayoutName(bool simpleName) const noexcept {
    return AudioChannelLayoutName(channelLayout_, simpleName);
}

inline NSString *_Nullable CAChannelLayout::LayoutDescription() const noexcept {
    return AudioChannelLayoutDescription(channelLayout_);
}
#endif /* __OBJC__ */

inline const AudioChannelLayout *_Nullable CAChannelLayout::get() const noexcept {
    return channelLayout_;
}

inline void CAChannelLayout::reset(AudioChannelLayout *_Nullable channelLayout) noexcept {
    std::free(std::exchange(channelLayout_, channelLayout));
}

inline void CAChannelLayout::swap(CAChannelLayout& other) noexcept {
    std::swap(channelLayout_, other.channelLayout_);
}

inline AudioChannelLayout *_Nullable CAChannelLayout::release() noexcept {
    return std::exchange(channelLayout_, nullptr);
}

} /* namespace CXXCoreAudio */
