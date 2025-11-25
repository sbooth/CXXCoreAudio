//
// Copyright © 2025 Stephen F. Booth
// Part of https://github.com/sbooth/CXXCoreAudio
// MIT license
//

#pragma once

#ifdef __OBJC__
#import <AVFAudio/AVFAudio.h>
#endif /* __OBJC__ */

#import <CXXCoreAudio/CAStreamDescription.hpp>
#import <CXXCoreAudio/CAChannelLayout.hpp>

namespace CXXCoreAudio {

/// A class representing an audio format with both a stream description and channel layout.
class CAAudioFormat final {
public:
	CAAudioFormat() noexcept = delete;
	CAAudioFormat(const CAAudioFormat& other) = default;
	CAAudioFormat(CAAudioFormat&& other) = default;
	CAAudioFormat& operator=(const CAAudioFormat& other) = default;
	CAAudioFormat& operator=(CAAudioFormat&& other) = default;
	~CAAudioFormat() noexcept = default;

	/// Creates an audio format with the specified stream description and no channel layout.
	///
	/// It is an error to specify a stream description with more than two channels.
	/// @throw std::invalid_argument
	explicit CAAudioFormat(const AudioStreamBasicDescription& format);

	/// Creates an audio format with the specified stream description and channel layout.
	///
	/// It is an error to specify a stream description with more than two channels with a null channel layout,
	/// or a stream description and channel layout with unequal channel counts.
	/// @throw std::invalid_argument
	CAAudioFormat(const AudioStreamBasicDescription& format, const AudioChannelLayout *layout);

	/// Returns true if this audio format is equal to another.
	bool operator==(const CAAudioFormat& other) const noexcept
	{ return streamDescription_ == other.streamDescription_ && channelLayout_ == other.channelLayout_; }

	/// Returns true if this audio format is not equal to another.
	bool operator!=(const CAAudioFormat& other) const noexcept
	{ return streamDescription_ != other.streamDescription_ || channelLayout_ != other.channelLayout_; }

	/// Returns the format's stream description.
	const CAStreamDescription& StreamDescription() const noexcept
	{ return streamDescription_; }

	/// Returns the format's channel layout.
	const CAChannelLayout& ChannelLayout() const noexcept
	{ return channelLayout_; }

#ifdef __OBJC__
	AVAudioFormat * AVAudioFormat() const noexcept
	{
		return [[AVAudioFormat alloc] initWithStreamDescription:&streamDescription_ channelLayout:channelLayout_];
	}
#endif /* __OBJC__ */

private:
	/// The format's stream description.
	CAStreamDescription streamDescription_{};
	/// The format's channel layout.
	CAChannelLayout channelLayout_{};
};

} /* namespace CXXCoreAudio */
