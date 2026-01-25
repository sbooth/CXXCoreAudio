//
// Copyright © 2025 Stephen F. Booth
// Part of https://github.com/sbooth/CXXCoreAudio
// MIT license
//

#import "CAAudioFormat.hpp"

#import <stdexcept>

CXXCoreAudio::CAAudioFormat::CAAudioFormat(const AudioStreamBasicDescription& format)
  : streamDescription_{format} {
    if (streamDescription_.mChannelsPerFrame > 2) {
        throw std::invalid_argument("More than two channels requires a channel layout");
    }

    if (streamDescription_.mChannelsPerFrame == 1) {
        channelLayout_ = CAChannelLayout::Mono;
    } else if (streamDescription_.mChannelsPerFrame == 2) {
        channelLayout_ = CAChannelLayout::Stereo;
    }
}

CXXCoreAudio::CAAudioFormat::CAAudioFormat(const AudioStreamBasicDescription& format, const AudioChannelLayout *layout)
  : streamDescription_{format}, channelLayout_{layout} {
    if (streamDescription_.mChannelsPerFrame > 2 && !channelLayout_) {
        throw std::invalid_argument("More than two channels requires a channel layout");
    }

    if (!channelLayout_) {
        if (streamDescription_.mChannelsPerFrame == 1) {
            channelLayout_ = CAChannelLayout::Mono;
        } else if (streamDescription_.mChannelsPerFrame == 2) {
            channelLayout_ = CAChannelLayout::Stereo;
        }
    }

    if (streamDescription_.mChannelsPerFrame != channelLayout_.ChannelCount()) {
        throw std::invalid_argument("Channel count mismatch between stream description and channel layout");
    }
}
