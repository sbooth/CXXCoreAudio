//
// SPDX-FileCopyrightText: 2013 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXCoreAudio
//

#include "core_audio/ChannelLayout.hpp"

#include <AudioToolbox/AudioFormat.h>

#include <cstring>
#include <new>

namespace {

/// Returns the string representation of an AudioChannelLayoutTag.
constexpr const char *getChannelLayoutTagName(AudioChannelLayoutTag layoutTag) noexcept {
    switch (layoutTag) {
    case kAudioChannelLayoutTag_UseChannelDescriptions:
        return "Use Channel Descriptions";
    case kAudioChannelLayoutTag_UseChannelBitmap:
        return "Use Channel Bitmap";

    case kAudioChannelLayoutTag_Mono:
        return "Mono";
    case kAudioChannelLayoutTag_Stereo:
        return "Stereo";
    case kAudioChannelLayoutTag_StereoHeadphones:
        return "Stereo Headphones";
    case kAudioChannelLayoutTag_MatrixStereo:
        return "Matrix Stereo";
    case kAudioChannelLayoutTag_MidSide:
        return "Mid-Side";
    case kAudioChannelLayoutTag_XY:
        return "XY";
    case kAudioChannelLayoutTag_Binaural:
        return "Binaural";
    case kAudioChannelLayoutTag_Ambisonic_B_Format:
        return "Ambisonic B-format";
    case kAudioChannelLayoutTag_Quadraphonic:
        return "Quadraphonic";
    case kAudioChannelLayoutTag_Pentagonal:
        return "Pentagonal";
    case kAudioChannelLayoutTag_Hexagonal:
        return "Hexagonal";
    case kAudioChannelLayoutTag_Octagonal:
        return "Octagonal";
    case kAudioChannelLayoutTag_Cube:
        return "Cube";
    case kAudioChannelLayoutTag_MPEG_3_0_A:
        return "MPEG 3.0 A";
    case kAudioChannelLayoutTag_MPEG_3_0_B:
        return "MPEG 3.0 B";
    case kAudioChannelLayoutTag_MPEG_4_0_A:
        return "MPEG 4.0 A";
    case kAudioChannelLayoutTag_MPEG_4_0_B:
        return "MPEG 4.0 B";
    case kAudioChannelLayoutTag_MPEG_5_0_A:
        return "MPEG 5.0 A";
    case kAudioChannelLayoutTag_MPEG_5_0_B:
        return "MPEG 5.0 B";
    case kAudioChannelLayoutTag_MPEG_5_0_C:
        return "MPEG 5.0 C";
    case kAudioChannelLayoutTag_MPEG_5_0_D:
        return "MPEG 5.0 D";
    case kAudioChannelLayoutTag_MPEG_5_1_A:
        return "MPEG 5.1 A";
    case kAudioChannelLayoutTag_MPEG_5_1_B:
        return "MPEG 5.1 B";
    case kAudioChannelLayoutTag_MPEG_5_1_C:
        return "MPEG 5.1 C";
    case kAudioChannelLayoutTag_MPEG_5_1_D:
        return "MPEG 5.1 D";
    case kAudioChannelLayoutTag_MPEG_6_1_A:
        return "MPEG 6.1 A";
    case kAudioChannelLayoutTag_MPEG_7_1_A:
        return "MPEG 7.1 A";
    case kAudioChannelLayoutTag_MPEG_7_1_B:
        return "MPEG 7.1 B";
    case kAudioChannelLayoutTag_MPEG_7_1_C:
        return "MPEG 7.1 C";
    case kAudioChannelLayoutTag_Emagic_Default_7_1:
        return "Emagic Default 7.1";
    case kAudioChannelLayoutTag_SMPTE_DTV:
        return "SMPTE DTV";
    case kAudioChannelLayoutTag_ITU_2_1:
        return "ITU 2.1";
    case kAudioChannelLayoutTag_ITU_2_2:
        return "ITU 2.2";
    case kAudioChannelLayoutTag_DVD_4:
        return "DVD 4";
    case kAudioChannelLayoutTag_DVD_5:
        return "DVD 5";
    case kAudioChannelLayoutTag_DVD_6:
        return "DVD 6";
    case kAudioChannelLayoutTag_DVD_10:
        return "DVD 10";
    case kAudioChannelLayoutTag_DVD_11:
        return "DVD 11";
    case kAudioChannelLayoutTag_DVD_18:
        return "DVD 18";
    case kAudioChannelLayoutTag_AudioUnit_6_0:
        return "AudioUnit 6.0";
    case kAudioChannelLayoutTag_AudioUnit_7_0:
        return "AudioUnit 7.0";
    case kAudioChannelLayoutTag_AudioUnit_7_0_Front:
        return "AudioUnit 7.0 Front";
    case kAudioChannelLayoutTag_AAC_6_0:
        return "AAC 6.0";
    case kAudioChannelLayoutTag_AAC_6_1:
        return "AAC 6.1";
    case kAudioChannelLayoutTag_AAC_7_0:
        return "AAC 7.0";
    case kAudioChannelLayoutTag_AAC_7_1_B:
        return "AAC 7.1 B";
    case kAudioChannelLayoutTag_AAC_7_1_C:
        return "AAC 7.1 C";
    case kAudioChannelLayoutTag_AAC_Octagonal:
        return "AAC Octagonal";
    case kAudioChannelLayoutTag_TMH_10_2_std:
        return "TMH 10.2 standard";
    case kAudioChannelLayoutTag_TMH_10_2_full:
        return "TMH 10.2 full";
    case kAudioChannelLayoutTag_AC3_1_0_1:
        return "AC-3 1.0.1";
    case kAudioChannelLayoutTag_AC3_3_0:
        return "AC-3 3.0";
    case kAudioChannelLayoutTag_AC3_3_1:
        return "AC-3 3.1";
    case kAudioChannelLayoutTag_AC3_3_0_1:
        return "AC-3 3.0.1";
    case kAudioChannelLayoutTag_AC3_2_1_1:
        return "AC-3 2.1.1";
    case kAudioChannelLayoutTag_AC3_3_1_1:
        return "AC-3 3.1.1";
    case kAudioChannelLayoutTag_EAC_6_0_A:
        return "EAC 6.0 A";
    case kAudioChannelLayoutTag_EAC_7_0_A:
        return "EAC 7.0 A";
    case kAudioChannelLayoutTag_EAC3_6_1_A:
        return "E-AC-3 6.1 A";
    case kAudioChannelLayoutTag_EAC3_6_1_B:
        return "E-AC-3 6.1 B";
    case kAudioChannelLayoutTag_EAC3_6_1_C:
        return "E-AC-3 6.1 C";
    case kAudioChannelLayoutTag_EAC3_7_1_A:
        return "E-AC-3 7.1 A";
    case kAudioChannelLayoutTag_EAC3_7_1_B:
        return "E-AC-3 7.1 B";
    case kAudioChannelLayoutTag_EAC3_7_1_C:
        return "E-AC-3 7.1 C";
    case kAudioChannelLayoutTag_EAC3_7_1_D:
        return "E-AC-3 7.1 D";
    case kAudioChannelLayoutTag_EAC3_7_1_E:
        return "E-AC-3 7.1 E";
    case kAudioChannelLayoutTag_EAC3_7_1_F:
        return "E-AC-3 7.1 F";
    case kAudioChannelLayoutTag_EAC3_7_1_G:
        return "E-AC-3 7.1 G";
    case kAudioChannelLayoutTag_EAC3_7_1_H:
        return "E-AC-3 7.1 H";
    case kAudioChannelLayoutTag_DTS_3_1:
        return "DTS 3.1";
    case kAudioChannelLayoutTag_DTS_4_1:
        return "DTS 4.1";
    case kAudioChannelLayoutTag_DTS_6_0_A:
        return "DTS 6.0 A";
    case kAudioChannelLayoutTag_DTS_6_0_B:
        return "DTS 6.0 B";
    case kAudioChannelLayoutTag_DTS_6_0_C:
        return "DTS 6.0 C";
    case kAudioChannelLayoutTag_DTS_6_1_A:
        return "DTS 6.1 A";
    case kAudioChannelLayoutTag_DTS_6_1_B:
        return "DTS 6.1 B";
    case kAudioChannelLayoutTag_DTS_6_1_C:
        return "DTS 6.1 C";
    case kAudioChannelLayoutTag_DTS_7_0:
        return "DTS 7.0";
    case kAudioChannelLayoutTag_DTS_7_1:
        return "DTS 7.1";
    case kAudioChannelLayoutTag_DTS_8_0_A:
        return "DTS 8.0 A";
    case kAudioChannelLayoutTag_DTS_8_0_B:
        return "DTS 8.0 B";
    case kAudioChannelLayoutTag_DTS_8_1_A:
        return "DTS 8.1 A";
    case kAudioChannelLayoutTag_DTS_8_1_B:
        return "DTS 8.1 B";
    case kAudioChannelLayoutTag_DTS_6_1_D:
        return "DTS 6.1 D";
    case kAudioChannelLayoutTag_WAVE_4_0_B:
        return "WAVE 4.0 B";
    case kAudioChannelLayoutTag_WAVE_5_0_B:
        return "WAVE 5.0 B";
    case kAudioChannelLayoutTag_WAVE_5_1_B:
        return "WAVE 5.1 B";
    case kAudioChannelLayoutTag_WAVE_6_1:
        return "WAVE 6.1";
    case kAudioChannelLayoutTag_WAVE_7_1:
        return "WAVE 7.1";
    case kAudioChannelLayoutTag_Atmos_5_1_2:
        return "Atmos 5.1.2";
    case kAudioChannelLayoutTag_Atmos_5_1_4:
        return "Atmos 5.1.4";
    case kAudioChannelLayoutTag_Atmos_7_1_2:
        return "Atmos 7.1.2";
    case kAudioChannelLayoutTag_Atmos_7_1_4:
        return "Atmos 7.1.4";
    case kAudioChannelLayoutTag_Atmos_9_1_6:
        return "Atmos 9.1.6";
    case kAudioChannelLayoutTag_Logic_4_0_C:
        return "Logic 4.0 C";
    case kAudioChannelLayoutTag_Logic_6_0_B:
        return "Logic 6.0 B";
    case kAudioChannelLayoutTag_Logic_6_1_B:
        return "Logic 6.1 B";
    case kAudioChannelLayoutTag_Logic_6_1_D:
        return "Logic 6.1 D";
    case kAudioChannelLayoutTag_Logic_7_1_B:
        return "Logic 7.1 B";
    case kAudioChannelLayoutTag_Logic_Atmos_7_1_4_B:
        return "Logic Atmos 7.1.4 B";
    case kAudioChannelLayoutTag_Logic_Atmos_7_1_6:
        return "Logic Atmos 7.1.6";
    case kAudioChannelLayoutTag_CICP_13:
        return "CICP 13";
    case kAudioChannelLayoutTag_CICP_14:
        return "CICP 14";
    case kAudioChannelLayoutTag_CICP_15:
        return "CICP 15";
    case kAudioChannelLayoutTag_CICP_16:
        return "CICP 16";
    case kAudioChannelLayoutTag_CICP_17:
        return "CICP 17";
    case kAudioChannelLayoutTag_CICP_18:
        return "CICP 18";
    case kAudioChannelLayoutTag_CICP_19:
        return "CICP 19";
    case kAudioChannelLayoutTag_CICP_20:
        return "CICP 20";
    case kAudioChannelLayoutTag_Ogg_5_0:
        return "Ogg 5.0";
    case kAudioChannelLayoutTag_Ogg_5_1:
        return "Ogg 5.1";
    case kAudioChannelLayoutTag_Ogg_6_1:
        return "Ogg 6.1";
    case kAudioChannelLayoutTag_Ogg_7_1:
        return "Ogg 7.1";
    case kAudioChannelLayoutTag_MPEG_5_0_E:
        return "MPEG 5.0 E";
    case kAudioChannelLayoutTag_MPEG_5_1_E:
        return "MPEG 5.1 E";
    case kAudioChannelLayoutTag_MPEG_6_1_B:
        return "MPEG 6.1 B";
    case kAudioChannelLayoutTag_MPEG_7_1_D:
        return "MPEG 7.1 D";

    default:
        break;
    }

    if (layoutTag >= kAudioChannelLayoutTag_BeginReserved && layoutTag <= kAudioChannelLayoutTag_EndReserved) {
        return "Reserved";
    }

    switch (layoutTag & 0xFFFF0000) {
    case kAudioChannelLayoutTag_HOA_ACN_SN3D:
        return "HOA ACN SN3D";
    case kAudioChannelLayoutTag_HOA_ACN_N3D:
        return "HOA ACN N3D";
    case kAudioChannelLayoutTag_DiscreteInOrder:
        return "Discrete in Order";
    case kAudioChannelLayoutTag_Unknown:
        return "Unknown";

    default:
        break;
    }

    return nullptr;
}

/// Returns the name of the channel for an AudioChannelLabel.
cf::CFString copyChannelLabelName(AudioChannelLabel channelLabel, bool shortName) noexcept {
    const auto property = shortName ? kAudioFormatProperty_ChannelShortName : kAudioFormatProperty_ChannelName;
    CFStringRef channelName = nullptr;
    UInt32 dataSize = sizeof channelName;
    OSStatus result = AudioFormatGetProperty(property, sizeof channelLabel, &channelLabel, &dataSize, &channelName);
    if (result != noErr) {
        return nullptr;
    }
    return cf::CFString(channelName);
}

/// Joins strings from array separated by delimiter.
cf::CFString joinStringArray(CFArrayRef array, CFStringRef delimiter) noexcept {
    if (array == nullptr) {
        return nullptr;
    }

    cf::CFMutableString result{CFStringCreateMutable(kCFAllocatorDefault, 0)};
    if (!result) {
        return nullptr;
    }

    const CFIndex count = CFArrayGetCount(array);
    for (CFIndex i = 0; i < count; ++i) {
        auto value = CFArrayGetValueAtIndex(array, i);
        // Skip non-strings
        if (CFGetTypeID(value) != CFStringGetTypeID()) {
            continue;
        }
        CFStringAppend(result.get(), static_cast<CFStringRef>(value));
        if (delimiter && i < count - 1) {
            CFStringAppend(result.get(), delimiter);
        }
    }

    return cf::CFString(result.leak());
}

} /* namespace */

// MARK: AudioChannelLayout Helper Functions

core_audio::malloc_ptr<AudioChannelLayout>
core_audio::allocateAudioChannelLayout(UInt32 numberChannelDescriptions) noexcept {
    const auto layoutSize = audioChannelLayoutSize(numberChannelDescriptions);
    auto channelLayout = static_cast<AudioChannelLayout *>(std::malloc(layoutSize));
    if (channelLayout == nullptr) {
        return nullptr;
    }
    std::memset(channelLayout, 0, layoutSize);
    channelLayout->mNumberChannelDescriptions = numberChannelDescriptions;
    return malloc_ptr<AudioChannelLayout>{channelLayout};
}

core_audio::malloc_ptr<AudioChannelLayout>
core_audio::copyAudioChannelLayout(const AudioChannelLayout *other) noexcept {
    if (other == nullptr) {
        return nullptr;
    }
    auto channelLayout = allocateAudioChannelLayout(other->mNumberChannelDescriptions);
    if (channelLayout) {
        std::memcpy(channelLayout.get(), other, audioChannelLayoutSize(other->mNumberChannelDescriptions));
    }
    return channelLayout;
}

UInt32 core_audio::audioChannelLayoutChannelCount(const AudioChannelLayout *channelLayout) noexcept {
    if (channelLayout == nullptr) {
        return 0;
    }
    if (channelLayout->mChannelLayoutTag == kAudioChannelLayoutTag_UseChannelDescriptions) {
        return channelLayout->mNumberChannelDescriptions;
    }
    if (channelLayout->mChannelLayoutTag == kAudioChannelLayoutTag_UseChannelBitmap) {
        return static_cast<UInt32>(__builtin_popcount(channelLayout->mChannelBitmap));
    }
    return AudioChannelLayoutTag_GetNumberOfChannels(channelLayout->mChannelLayoutTag);
}

bool core_audio::audioChannelLayoutsAreEqual(const AudioChannelLayout *lhs, const AudioChannelLayout *rhs) noexcept {
    if (lhs == nullptr && rhs == nullptr) {
        return true;
    }
    if ((lhs != nullptr && rhs == nullptr) || (lhs == nullptr && rhs != nullptr)) {
        return false;
    }

    const auto lsize = audioChannelLayoutSize(lhs->mNumberChannelDescriptions);
    const auto rsize = audioChannelLayoutSize(rhs->mNumberChannelDescriptions);
    if (lsize != rsize) {
        return false;
    }
    return !memcmp(lhs, rhs, lsize);
}

bool core_audio::audioChannelLayoutsAreEquivalent(const AudioChannelLayout *lhs,
                                                  const AudioChannelLayout *rhs) noexcept {
    if (lhs == nullptr && rhs == nullptr) {
        return true;
    }
    if (lhs != nullptr && rhs == nullptr) {
        if (const auto tag = lhs->mChannelLayoutTag;
            tag == kAudioChannelLayoutTag_Mono || tag == kAudioChannelLayoutTag_Stereo) {
            return true;
        }
    } else if (lhs == nullptr && rhs != nullptr) {
        if (const auto tag = rhs->mChannelLayoutTag;
            tag == kAudioChannelLayoutTag_Mono || tag == kAudioChannelLayoutTag_Stereo) {
            return true;
        }
    }

    if (lhs == nullptr || rhs == nullptr) {
        return false;
    }

    const AudioChannelLayout *layouts[] = {
            lhs,
            rhs,
    };
    UInt32 layoutsEquivalent = 0;
    UInt32 propertySize = sizeof layoutsEquivalent;
    OSStatus result = AudioFormatGetProperty(kAudioFormatProperty_AreChannelLayoutsEquivalent, sizeof layouts,
                                             static_cast<const void *>(layouts), &propertySize, &layoutsEquivalent);
    if (result != noErr) {
        return false;
    }
    return layoutsEquivalent;
}

cf::CFString core_audio::copyAudioChannelLayoutName(const AudioChannelLayout *channelLayout,
                                                          bool simpleName) noexcept {
    if (channelLayout == nullptr) {
        return nullptr;
    }
    const auto property =
            simpleName ? kAudioFormatProperty_ChannelLayoutSimpleName : kAudioFormatProperty_ChannelLayoutName;
    CFStringRef layoutName = nullptr;
    UInt32 dataSize = sizeof layoutName;
    OSStatus result = AudioFormatGetProperty(property, static_cast<UInt32>(audioChannelLayoutSize(channelLayout)),
                                             channelLayout, &dataSize, &layoutName);
    if (result != noErr) {
        return nullptr;
    }
    return cf::CFString(layoutName);
}

cf::CFString core_audio::copyAudioChannelLayoutDescription(const AudioChannelLayout *channelLayout) noexcept {
    if (channelLayout == nullptr) {
        return nullptr;
    }

    cf::CFMutableString result{CFStringCreateMutable(kCFAllocatorDefault, 0)};
    if (!result) {
        return nullptr;
    }

    auto layoutName = copyAudioChannelLayoutName(channelLayout);

    if (channelLayout->mChannelLayoutTag == kAudioChannelLayoutTag_UseChannelDescriptions) {
        // kAudioFormatProperty_ChannelLayoutName returns '!fmt' for kAudioChannelLabel_UseCoordinates
        if (layoutName) {
            CFStringAppendFormat(result, nullptr, CFSTR("%u channel descriptions, %@"),
                                 channelLayout->mNumberChannelDescriptions, layoutName.get());
            return cf::CFString(result.leak());
        }

        CFStringAppendFormat(result, nullptr, CFSTR("%u channel descriptions"),
                             channelLayout->mNumberChannelDescriptions);

        cf::CFMutableArray array{CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks)};
        if (!array) {
            // Allocation failed; return the partial description without per-channel names
            return cf::CFString(result.leak());
        }

        const AudioChannelDescription *desc = channelLayout->mChannelDescriptions;
        for (UInt32 i = 0; i < channelLayout->mNumberChannelDescriptions; ++i, ++desc) {
            if (desc->mChannelLabel == kAudioChannelLabel_UseCoordinates) {
                cf::CFString coordinateString;
                if (desc->mChannelFlags & kAudioChannelFlags_RectangularCoordinates) {
                    coordinateString.reset(CFStringCreateWithFormat(
                            kCFAllocatorDefault, nullptr, CFSTR("[x: %g, y: %g, z: %g%s]"), desc->mCoordinates[0],
                            desc->mCoordinates[1], desc->mCoordinates[2],
                            desc->mChannelFlags & kAudioChannelFlags_Meters ? " m" : ""));
                } else if (desc->mChannelFlags & kAudioChannelFlags_SphericalCoordinates) {
                    coordinateString.reset(CFStringCreateWithFormat(
                            kCFAllocatorDefault, nullptr, CFSTR("[r: %g, θ: %g, φ: %g%s]"), desc->mCoordinates[2],
                            desc->mCoordinates[1], desc->mCoordinates[0],
                            desc->mChannelFlags & kAudioChannelFlags_Meters ? " m" : ""));
                } else {
                    coordinateString.reset(CFStringCreateWithFormat(
                            kCFAllocatorDefault, nullptr, CFSTR("[?! %g, %g, %g%s]"), desc->mCoordinates[0],
                            desc->mCoordinates[1], desc->mCoordinates[2],
                            desc->mChannelFlags & kAudioChannelFlags_Meters ? " m" : ""));
                }

                if (coordinateString) {
                    CFArrayAppendValue(array, coordinateString.get());
                }
            } else {
                if (const auto channelName = copyChannelLabelName(desc->mChannelLabel, true); channelName) {
                    CFArrayAppendValue(array, channelName.get());
                } else {
                    CFArrayAppendValue(array, CFSTR("?"));
                }
            }
        }

        if (auto channelNamesString = joinStringArray(array, CFSTR(" ")); channelNamesString) {
            CFStringAppendFormat(result, nullptr, CFSTR(", %@"), channelNamesString.get());
        }
    } else if (channelLayout->mChannelLayoutTag == kAudioChannelLayoutTag_UseChannelBitmap) {
        CFStringAppendFormat(result, nullptr, CFSTR("Bitmap %#x (%u ch)"), channelLayout->mChannelBitmap,
                             __builtin_popcount(channelLayout->mChannelBitmap));
        if (layoutName) {
            CFStringAppendFormat(result, nullptr, CFSTR(", %@"), layoutName.get());
        }
    } else {
        CFStringAppendFormat(result, nullptr, CFSTR("%s (0x%x, %u ch)"),
                             getChannelLayoutTagName(channelLayout->mChannelLayoutTag),
                             channelLayout->mChannelLayoutTag, channelLayout->mChannelLayoutTag & 0xffff);
        if (layoutName) {
            CFStringAppendFormat(result, nullptr, CFSTR(", %@"), layoutName.get());
        }
    }

    return cf::CFString(result.leak());
}

// Constants
const core_audio::ChannelLayout core_audio::ChannelLayout::Mono = ChannelLayout(kAudioChannelLayoutTag_Mono);
const core_audio::ChannelLayout core_audio::ChannelLayout::Stereo = ChannelLayout(kAudioChannelLayoutTag_Stereo);

core_audio::ChannelLayout core_audio::ChannelLayout::channelLayoutWithBitmap(AudioChannelBitmap channelBitmap) {
    ChannelLayout channelLayout{kAudioChannelLayoutTag_UseChannelBitmap};
    channelLayout.channelLayout_->mChannelBitmap = channelBitmap;

    // Attempt to convert the bitmap to a layout tag
    AudioChannelLayoutTag tag;
    UInt32 dataSize = sizeof tag;
    OSStatus result = AudioFormatGetProperty(kAudioFormatProperty_TagForChannelLayout,
                                             static_cast<UInt32>(audioChannelLayoutSize(channelLayout.channelLayout_)),
                                             channelLayout.channelLayout_, &dataSize, &tag);
    if (result == noErr) {
        channelLayout.channelLayout_->mChannelLayoutTag = tag;
        channelLayout.channelLayout_->mChannelBitmap = 0;
    }

    return channelLayout;
}

core_audio::ChannelLayout core_audio::ChannelLayout::channelLayoutWithTag(AudioChannelLayoutTag layoutTag) {
    return ChannelLayout{layoutTag};
}

core_audio::ChannelLayout
core_audio::ChannelLayout::channelLayoutWithChannelLabels(std::vector<AudioChannelLabel> channelLabels) {
    return ChannelLayout{channelLabels};
}

core_audio::ChannelLayout::ChannelLayout(const ChannelLayout &other) : ChannelLayout{} { *this = other; }

core_audio::ChannelLayout &core_audio::ChannelLayout::operator=(const ChannelLayout &other) {
    if (this != &other) {
        if (channelLayout_ != nullptr && other.channelLayout_ != nullptr &&
            channelLayout_->mNumberChannelDescriptions == other.channelLayout_->mNumberChannelDescriptions) {
            memcpy(channelLayout_, other.channelLayout_,
                   audioChannelLayoutSize(other.channelLayout_->mNumberChannelDescriptions));
        } else {
            auto channelLayout = copyAudioChannelLayout(other.channelLayout_);
            if (other.channelLayout_ != nullptr && channelLayout == nullptr) {
                throw std::bad_alloc();
            }
            reset(channelLayout.release());
        }
    }
    return *this;
}

core_audio::ChannelLayout::ChannelLayout(AudioChannelLayoutTag layoutTag)
    : channelLayout_{allocateAudioChannelLayout(0).release()} {
    if (channelLayout_ == nullptr) {
        throw std::bad_alloc();
    }
    channelLayout_->mChannelLayoutTag = layoutTag;
}

core_audio::ChannelLayout::ChannelLayout(std::vector<AudioChannelLabel> channelLabels)
    : channelLayout_{allocateAudioChannelLayout(static_cast<UInt32>(channelLabels.size())).release()} {
    if (channelLayout_ == nullptr) {
        throw std::bad_alloc();
    }

    channelLayout_->mChannelLayoutTag = kAudioChannelLayoutTag_UseChannelDescriptions;
    for (std::vector<AudioChannelLabel>::size_type i = 0; i != channelLabels.size(); ++i) {
        channelLayout_->mChannelDescriptions[i].mChannelLabel = channelLabels[i];
    }

    // Attempt to convert the channel labels to a layout tag
    AudioChannelLayoutTag tag;
    UInt32 dataSize = sizeof tag;
    OSStatus result = AudioFormatGetProperty(kAudioFormatProperty_TagForChannelLayout,
                                             static_cast<UInt32>(audioChannelLayoutSize(channelLayout_)),
                                             channelLayout_, &dataSize, &tag);
    if (result == noErr) {
        auto channelLayout = allocateAudioChannelLayout(0);
        if (!channelLayout) {
            throw std::bad_alloc();
        }
        reset(channelLayout.release());
        channelLayout_->mChannelLayoutTag = tag;
    }
}

core_audio::ChannelLayout::ChannelLayout(const AudioChannelLayout *other)
    : channelLayout_{copyAudioChannelLayout(other).release()} {
    if (other != nullptr && channelLayout_ == nullptr) {
        throw std::bad_alloc();
    }
}

core_audio::ChannelLayout &core_audio::ChannelLayout::operator=(const AudioChannelLayout *other) {
    auto channelLayout = copyAudioChannelLayout(other);
    if (other != nullptr && !channelLayout) {
        throw std::bad_alloc();
    }
    reset(channelLayout.release());
    return *this;
}

core_audio::ChannelLayout::ChannelLayout(ChannelLayout &&other) noexcept : channelLayout_{other.release()} {}

core_audio::ChannelLayout &core_audio::ChannelLayout::operator=(ChannelLayout &&other) noexcept {
    reset(other.release());
    return *this;
}

core_audio::ChannelLayout::~ChannelLayout() noexcept { reset(); }

// MARK: Functionality

bool core_audio::ChannelLayout::mapToLayout(const ChannelLayout &outputLayout, std::vector<SInt32> &channelMap) const {
    // No valid map exists for empty/unknown layouts
    if (channelLayout_ == nullptr || outputLayout.channelLayout_ == nullptr) {
        return false;
    }

    const AudioChannelLayout *layouts[] = {channelLayout_, outputLayout.channelLayout_};

    const auto outputChannelCount = outputLayout.channelCount();
    if (outputChannelCount == 0) {
        return false;
    }

    channelMap.resize(outputChannelCount);
    UInt32 propertySize = static_cast<UInt32>(sizeof(SInt32) * channelMap.size());
    OSStatus result = AudioFormatGetProperty(kAudioFormatProperty_ChannelMap, sizeof layouts,
                                             static_cast<const void *>(layouts), &propertySize, channelMap.data());
    return result == noErr;
}
