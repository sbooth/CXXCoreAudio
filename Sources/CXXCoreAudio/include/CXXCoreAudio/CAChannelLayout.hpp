//
// Copyright © 2013-2025 Stephen F. Booth
// Part of https://github.com/sbooth/CXXCoreAudio
// MIT license
//

#pragma once

#import <utility>
#import <vector>

#import <CoreAudioTypes/CoreAudioTypes.h>
#import <CoreFoundation/CFString.h>

#ifdef __OBJC__
#import <AVFAudio/AVFAudio.h>
#import <Foundation/NSString.h>
#endif /* __OBJC__ */

namespace CXXCoreAudio {

// MARK: AudioChannelLayout Helper Functions

/// Allocates and returns a new variable-length AudioChannelLayout structure with the specified number of channel descriptions.
/// @note The allocation is performed using std::malloc and should be deallocated using std::free.
/// @param numberChannelDescriptions The number of channel descriptions that will be stored in the channel layout.
/// @return An AudioChannelLayout struct or null if memory could not be allocated.
AudioChannelLayout * _Nullable AllocateAudioChannelLayout(UInt32 numberChannelDescriptions) noexcept;

/// Allocates and returns a copy of a variable-length AudioChannelLayout structure.
/// @note The allocation is performed using std::malloc and should be deallocated using std::free.
/// @param other The AudioChannelLayout to copy.
/// @return An AudioChannelLayout struct or null if memory could not be allocated.
AudioChannelLayout * _Nullable CopyAudioChannelLayout(const AudioChannelLayout * _Nullable other) noexcept;

/// Returns the size required to hold a variable-length AudioChannelLayout structure with the specified number of channel descriptions.
/// @return The required size in bytes.
constexpr size_t AudioChannelLayoutSize(UInt32 numberChannelDescriptions) noexcept
{
	return offsetof(AudioChannelLayout, mChannelDescriptions) + (numberChannelDescriptions * sizeof(AudioChannelDescription));
}

/// Returns the size of a variable-length AudioChannelLayout structure.
/// @return The size of the channel layout in bytes.
inline size_t AudioChannelLayoutSize(const AudioChannelLayout * _Nullable channelLayout) noexcept
{
	if(!channelLayout)
		return 0;
	return AudioChannelLayoutSize(channelLayout->mNumberChannelDescriptions);
}

/// Returns the number of channels contained in an audio channel layout.
UInt32 AudioChannelLayoutChannelCount(const AudioChannelLayout * _Nullable channelLayout) noexcept;

/// Returns true if two AudioChannelLayout structures are equal.
///
/// This function performs a bitwise comparison based on the number of channel descriptions.
/// @note Two equivalent channel layouts may not be equal.
/// @return true if the AudioChannelLayout structs are equal, false if not.
bool AudioChannelLayoutsAreEqual(const AudioChannelLayout * _Nullable lhs, const AudioChannelLayout * _Nullable rhs) noexcept;

/// Returns true if two AudioChannelLayout structures are equivalent.
///
/// Audio channel layouts are considered equivalent if:
/// 1) Both are null.
/// 2) One is null and the other has a mono or stereo layout tag.
/// 3) kAudioFormatProperty_AreChannelLayoutsEquivalent is true.
/// @note Two equivalent channel layouts may not be equal.
/// @return true if the AudioChannelLayout structs are equivalent, false if not.
bool AudioChannelLayoutsAreEquivalent(const AudioChannelLayout * _Nullable lhs, const AudioChannelLayout * _Nullable rhs) noexcept;

/// Returns true if two AudioChannelLayout structures are equal.
inline bool operator==(const AudioChannelLayout& lhs, const AudioChannelLayout& rhs) noexcept
{
	return AudioChannelLayoutsAreEqual(&lhs, &rhs);
}

/// Returns true if two AudioChannelLayout structures are not equal.
inline bool operator!=(const AudioChannelLayout& lhs, const AudioChannelLayout& rhs) noexcept
{
	return !operator==(lhs, rhs);
}

/// Returns the name of the channel layout described by an AudioChannelLayout structure.
///
/// This is the value of kAudioFormatProperty_ChannelLayoutName or kAudioFormatProperty_ChannelLayoutSimpleName.
/// @note The caller is responsible for releasing the returned string.
CFStringRef _Nullable CopyAudioChannelLayoutName(const AudioChannelLayout * _Nullable channelLayout, bool simpleName = false) noexcept CF_RETURNS_RETAINED;

/// Returns a string representation of the channel layout described by an AudioChannelLayout structure.
/// @note The caller is responsible for releasing the returned string.
CFStringRef _Nullable CopyAudioChannelLayoutDescription(const AudioChannelLayout * _Nullable channelLayout) noexcept CF_RETURNS_RETAINED;

#ifdef __OBJC__
/// Returns true if two the AVAudioChannelLayout objects are equivalent.
///
/// Audio channel layouts are considered equivalent if:
/// 1) Both are null.
/// 2) One is null and the other has a mono or stereo layout tag.
/// 3) kAudioFormatProperty_AreChannelLayoutsEquivalent is true.
/// @note Two equivalent channel layouts may not be equal.
/// @return true if the AudioChannelLayout structs are equivalent, false if not.
inline bool AVAudioChannelLayoutsAreEquivalent(AVAudioChannelLayout * _Nullable lhs, AVAudioChannelLayout * _Nullable rhs) noexcept
{
	return AudioChannelLayoutsAreEquivalent(lhs.layout, rhs.layout);
}

/// Returns the name of the channel layout described by an AudioChannelLayout structure.
///
/// This is the value of kAudioFormatProperty_ChannelLayoutName or kAudioFormatProperty_ChannelLayoutSimpleName.
inline NSString * _Nullable AudioChannelLayoutName(const AudioChannelLayout * _Nullable channelLayout, bool simpleName = false) noexcept
{
	return (__bridge_transfer NSString *)CopyAudioChannelLayoutName(channelLayout, simpleName);
}

/// Returns a string representation of the channel layout described by an AudioChannelLayout structure.
inline NSString * _Nullable AudioChannelLayoutDescription(const AudioChannelLayout * _Nullable channelLayout) noexcept
{
	return (__bridge_transfer NSString *)CopyAudioChannelLayoutDescription(channelLayout);
}
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
	CAChannelLayout(const AudioChannelLayout * _Nullable other);

	/// Replaces the channel layout with a copy of a channel layout.
	/// @throw std::bad_alloc if memory could not be allocated.
	CAChannelLayout& operator=(const CAChannelLayout& other);

	/// Replaces the channel layout with a copy of an AudioChannelLayout.
	/// @throw std::bad_alloc if memory could not be allocated.
	CAChannelLayout& operator=(const AudioChannelLayout * _Nullable other);

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
	bool IsEqual(const AudioChannelLayout * _Nullable other) const noexcept
	{
		return AudioChannelLayoutsAreEqual(channelLayout_, other);
	}

	/// Returns true if the channel layout is equal to another channel layout.
	///
	/// This function performs a bitwise comparison based on the number of channel descriptions.
	/// @note Two equivalent channel layouts may not be equal.
	bool IsEqual(const CAChannelLayout& other) const noexcept
	{
		return IsEqual(other.channelLayout_);
	}

	/// Returns true if the channel layout is equal to an AudioChannelLayout.
	bool operator==(const AudioChannelLayout * _Nullable other) const noexcept
	{
		return IsEqual(other);
	}

	/// Returns true if the channel layout is not equal to an AudioChannelLayout.
	bool operator!=(const AudioChannelLayout * _Nullable other) const noexcept
	{
		return !operator==(other);
	}

	/// Returns true if the channel layout is equal to another.
	bool operator==(const CAChannelLayout& other) const noexcept
	{
		return operator==(other.channelLayout_);
	}

	/// Returns true if the channel layout is not equal to another.
	bool operator!=(const CAChannelLayout& other) const noexcept
	{
		return !operator==(other.channelLayout_);
	}

	// MARK: Equivalence

	/// Returns true if the channel layout is equivalent to an AudioChannelLayout.
	///
	/// Channel layouts are considered equivalent if:
	/// 1) Both are empty.
	/// 2) One is empty and the other has a mono or stereo layout tag.
	/// 3) kAudioFormatProperty_AreChannelLayoutsEquivalent is true.
	/// @note Two equivalent channel layouts may not be equal.
	bool IsEquivalent(const AudioChannelLayout * _Nullable other) const noexcept
	{
		return AudioChannelLayoutsAreEquivalent(channelLayout_, other);
	}

	/// Returns true if the channel layout is equivalent to another channel layout.
	///
	/// Channel layouts are considered equivalent if:
	/// 1) Both are empty.
	/// 2) One is empty and the other has a mono or stereo layout tag.
	/// 3) kAudioFormatProperty_AreChannelLayoutsEquivalent is true.
	/// @note Two equivalent channel layouts may not be equal.
	bool IsEquivalent(const CAChannelLayout& other) const noexcept
	{
		return IsEquivalent(other.channelLayout_);
	}

	// MARK: Functionality

	/// Returns the number of channels contained in this channel layout.
	UInt32 ChannelCount() const noexcept
	{
		return AudioChannelLayoutChannelCount(channelLayout_);
	}

	/// Creates a channel map for remapping audio from this channel layout.
	/// @param outputLayout The output channel layout
	/// @param channelMap A std::vector to receive the channel map on success
	/// @return true on success, false otherwise
	/// @throw std::bad_alloc if memory could not be allocated.
	bool MapToLayout(const CAChannelLayout& outputLayout, std::vector<SInt32>& channelMap) const;

	// MARK: AudioChannelLayout access

	/// Returns the size in bytes of the managed AudioChannelLayout struct.
	size_t Size() const noexcept
	{
		return AudioChannelLayoutSize(channelLayout_);
	}

	/// A channel layout is empty when the managed AudioChannelLayout struct is null.
	explicit operator bool() const noexcept
	{
		return channelLayout_ != nullptr;
	}

	/// Returns a const pointer to the managed AudioChannelLayout struct.
	const AudioChannelLayout * _Nullable operator->() const noexcept
	{
		return channelLayout_;
	}

	/// Returns a const pointer to the managed AudioChannelLayout struct.
	operator const AudioChannelLayout * const _Nullable () const noexcept
	{
		return channelLayout_;
	}

	// MARK: Channel Layout Name and Description

	/// Returns the name of this channel layout.
	///
	/// This is the value of kAudioFormatProperty_ChannelLayoutName or kAudioFormatProperty_ChannelLayoutSimpleName.
	/// @note The caller is responsible for releasing the returned string
	CFStringRef _Nullable CopyLayoutName(bool simpleName = false) const noexcept CF_RETURNS_RETAINED
	{
		return CopyAudioChannelLayoutName(channelLayout_, simpleName);
	}

	/// Returns a string representation of this channel layout
	///
	/// This is the value of kAudioFormatProperty_ChannelLayoutName or kAudioFormatProperty_ChannelLayoutSimpleName.
	/// @note The caller is responsible for releasing the returned string
	CFStringRef _Nullable CopyLayoutDescription() const noexcept CF_RETURNS_RETAINED
	{
		return CopyAudioChannelLayoutDescription(channelLayout_);
	}

#ifdef __OBJC__
	/// Returns an AVAudioChannelLayout object initialized with the managed AudioChannelLayout struct.
	operator AVAudioChannelLayout * _Nullable () const noexcept
	{
		return [[AVAudioChannelLayout alloc] initWithLayout:channelLayout_];
	}

	/// Returns the name of this channel layout.
	NSString * _Nullable LayoutName(bool simpleName = false) const noexcept
	{
		return (__bridge_transfer NSString *)CopyLayoutName(simpleName);
	}

	/// Returns a string representation of this channel layout.
	NSString * _Nullable LayoutDescription() const noexcept
	{
		return (__bridge_transfer NSString *)CopyLayoutDescription();
	}
#endif /* __OBJC__ */


	/// Returns the managed AudioChannelLayout struct.
	const AudioChannelLayout * _Nullable get() const noexcept
	{
		return channelLayout_;
	}

	/// Replaces the object's managed AudioChannelLayout struct with another AudioChannelLayout struct.
	/// @note The object assumes responsibility for deallocating the passed AudioChannelLayout struct using std::free.
	void reset(AudioChannelLayout * _Nullable channelLayout = nullptr) noexcept
	{
		std::free(std::exchange(channelLayout_, channelLayout));
	}

	/// Swaps the managed AudioChannelLayout struct with the managed AudioChannelLayout struct from another audio channel layout.
	void swap(CAChannelLayout& other) noexcept
	{
		std::swap(channelLayout_, other.channelLayout_);
	}

	/// Releases ownership of the object's managed AudioChannelLayout struct and returns it.
	/// @note The caller assumes responsibility for deallocating the returned AudioChannelLayout struct using std::free.
	AudioChannelLayout * _Nullable release() noexcept
	{
		return std::exchange(channelLayout_, nullptr);
	}

private:
	/// The managed AudioChannelLayout structure.
	AudioChannelLayout * _Nullable channelLayout_{nullptr};
};

} /* namespace CXXCoreAudio */
