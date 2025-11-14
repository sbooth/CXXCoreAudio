//
// Copyright © 2013-2025 Stephen F. Booth
// Part of https://github.com/sbooth/CXXCoreAudio
// MIT license
//

#import <cstdlib>
#import <cstring>
#import <new>

#import <AudioToolbox/AudioFormat.h>

#import "CAChannelLayout.hpp"

namespace {

/// Returns the string representation of an AudioChannelLayoutTag.
constexpr const char * GetChannelLayoutTagName(AudioChannelLayoutTag layoutTag) noexcept
{
	switch(layoutTag) {
		case kAudioChannelLayoutTag_UseChannelDescriptions:
			return "Use Channel Descriptions";
		case kAudioChannelLayoutTag_UseChannelBitmap:
			return "Use Channel Bitmap";

		case kAudioChannelLayoutTag_Mono:					return "Mono";
		case kAudioChannelLayoutTag_Stereo:					return "Stereo";
		case kAudioChannelLayoutTag_StereoHeadphones:		return "Stereo Headphones";
		case kAudioChannelLayoutTag_MatrixStereo:			return "Matrix Stereo";
		case kAudioChannelLayoutTag_MidSide:				return "Mid-Side";
		case kAudioChannelLayoutTag_XY:						return "XY";
		case kAudioChannelLayoutTag_Binaural:				return "Binaural";
		case kAudioChannelLayoutTag_Ambisonic_B_Format:		return "Ambisonic B-format";
		case kAudioChannelLayoutTag_Quadraphonic:			return "Quadraphonic";
		case kAudioChannelLayoutTag_Pentagonal:				return "Pentagonal";
		case kAudioChannelLayoutTag_Hexagonal:				return "Hexagonal";
		case kAudioChannelLayoutTag_Octagonal:				return "Octagonal";
		case kAudioChannelLayoutTag_Cube:					return "Cube";
		case kAudioChannelLayoutTag_MPEG_3_0_A:				return "MPEG 3.0 A";
		case kAudioChannelLayoutTag_MPEG_3_0_B:				return "MPEG 3.0 B";
		case kAudioChannelLayoutTag_MPEG_4_0_A:				return "MPEG 4.0 A";
		case kAudioChannelLayoutTag_MPEG_4_0_B:				return "MPEG 4.0 B";
		case kAudioChannelLayoutTag_MPEG_5_0_A:				return "MPEG 5.0 A";
		case kAudioChannelLayoutTag_MPEG_5_0_B:				return "MPEG 5.0 B";
		case kAudioChannelLayoutTag_MPEG_5_0_C:				return "MPEG 5.0 C";
		case kAudioChannelLayoutTag_MPEG_5_0_D:				return "MPEG 5.0 D";
		case kAudioChannelLayoutTag_MPEG_5_1_A:				return "MPEG 5.1 A";
		case kAudioChannelLayoutTag_MPEG_5_1_B:				return "MPEG 5.1 B";
		case kAudioChannelLayoutTag_MPEG_5_1_C:				return "MPEG 5.1 C";
		case kAudioChannelLayoutTag_MPEG_5_1_D:				return "MPEG 5.1 D";
		case kAudioChannelLayoutTag_MPEG_6_1_A:				return "MPEG 6.1 A";
		case kAudioChannelLayoutTag_MPEG_7_1_A:				return "MPEG 7.1 A";
		case kAudioChannelLayoutTag_MPEG_7_1_B:				return "MPEG 7.1 B";
		case kAudioChannelLayoutTag_MPEG_7_1_C:				return "MPEG 7.1 C";
		case kAudioChannelLayoutTag_Emagic_Default_7_1:		return "Emagic Default 7.1";
		case kAudioChannelLayoutTag_SMPTE_DTV:				return "SMPTE DTV";
		case kAudioChannelLayoutTag_ITU_2_1:				return "ITU 2.1";
		case kAudioChannelLayoutTag_ITU_2_2:				return "ITU 2.2";
		case kAudioChannelLayoutTag_DVD_4:					return "DVD 4";
		case kAudioChannelLayoutTag_DVD_5:					return "DVD 5";
		case kAudioChannelLayoutTag_DVD_6:					return "DVD 6";
		case kAudioChannelLayoutTag_DVD_10:					return "DVD 10";
		case kAudioChannelLayoutTag_DVD_11:					return "DVD 11";
		case kAudioChannelLayoutTag_DVD_18:					return "DVD 18";
		case kAudioChannelLayoutTag_AudioUnit_6_0:			return "AudioUnit 6.0";
		case kAudioChannelLayoutTag_AudioUnit_7_0:			return "AudioUnit 7.0";
		case kAudioChannelLayoutTag_AudioUnit_7_0_Front:	return "AudioUnit 7.0 Front";
		case kAudioChannelLayoutTag_AAC_6_0:				return "AAC 6.0";
		case kAudioChannelLayoutTag_AAC_6_1:				return "AAC 6.1";
		case kAudioChannelLayoutTag_AAC_7_0:				return "AAC 7.0";
		case kAudioChannelLayoutTag_AAC_7_1_B:				return "AAC 7.1 B";
		case kAudioChannelLayoutTag_AAC_7_1_C:				return "AAC 7.1 C";
		case kAudioChannelLayoutTag_AAC_Octagonal:			return "AAC Octagonal";
		case kAudioChannelLayoutTag_TMH_10_2_std:			return "TMH 10.2 standard";
		case kAudioChannelLayoutTag_TMH_10_2_full:			return "TMH 10.2 full";
		case kAudioChannelLayoutTag_AC3_1_0_1:				return "AC-3 1.0.1";
		case kAudioChannelLayoutTag_AC3_3_0:				return "AC-3 3.0";
		case kAudioChannelLayoutTag_AC3_3_1:				return "AC-3 3.1";
		case kAudioChannelLayoutTag_AC3_3_0_1:				return "AC-3 3.0.1";
		case kAudioChannelLayoutTag_AC3_2_1_1:				return "AC-3 2.1.1";
		case kAudioChannelLayoutTag_AC3_3_1_1:				return "AC-3 3.1.1";
		case kAudioChannelLayoutTag_EAC_6_0_A:				return "EAC 6.0 A";
		case kAudioChannelLayoutTag_EAC_7_0_A:				return "EAC 7.0 A";
		case kAudioChannelLayoutTag_EAC3_6_1_A:				return "E-AC-3 6.1 A";
		case kAudioChannelLayoutTag_EAC3_6_1_B:				return "E-AC-3 6.1 B";
		case kAudioChannelLayoutTag_EAC3_6_1_C:				return "E-AC-3 6.1 C";
		case kAudioChannelLayoutTag_EAC3_7_1_A:				return "E-AC-3 7.1 A";
		case kAudioChannelLayoutTag_EAC3_7_1_B:				return "E-AC-3 7.1 B";
		case kAudioChannelLayoutTag_EAC3_7_1_C:				return "E-AC-3 7.1 C";
		case kAudioChannelLayoutTag_EAC3_7_1_D:				return "E-AC-3 7.1 D";
		case kAudioChannelLayoutTag_EAC3_7_1_E:				return "E-AC-3 7.1 E";
		case kAudioChannelLayoutTag_EAC3_7_1_F:				return "E-AC-3 7.1 F";
		case kAudioChannelLayoutTag_EAC3_7_1_G:				return "E-AC-3 7.1 G";
		case kAudioChannelLayoutTag_EAC3_7_1_H:				return "E-AC-3 7.1 H";
		case kAudioChannelLayoutTag_DTS_3_1:				return "DTS 3.1";
		case kAudioChannelLayoutTag_DTS_4_1:				return "DTS 4.1";
		case kAudioChannelLayoutTag_DTS_6_0_A:				return "DTS 6.0 A";
		case kAudioChannelLayoutTag_DTS_6_0_B:				return "DTS 6.0 B";
		case kAudioChannelLayoutTag_DTS_6_0_C:				return "DTS 6.0 C";
		case kAudioChannelLayoutTag_DTS_6_1_A:				return "DTS 6.1 A";
		case kAudioChannelLayoutTag_DTS_6_1_B:				return "DTS 6.1 B";
		case kAudioChannelLayoutTag_DTS_6_1_C:				return "DTS 6.1 C";
		case kAudioChannelLayoutTag_DTS_7_0:				return "DTS 7.0";
		case kAudioChannelLayoutTag_DTS_7_1:				return "DTS 7.1";
		case kAudioChannelLayoutTag_DTS_8_0_A:				return "DTS 8.0 A";
		case kAudioChannelLayoutTag_DTS_8_0_B:				return "DTS 8.0 B";
		case kAudioChannelLayoutTag_DTS_8_1_A:				return "DTS 8.1 A";
		case kAudioChannelLayoutTag_DTS_8_1_B:				return "DTS 8.1 B";
		case kAudioChannelLayoutTag_DTS_6_1_D:				return "DTS 6.1 D";
		case kAudioChannelLayoutTag_WAVE_4_0_B:				return "WAVE 4.0 B";
		case kAudioChannelLayoutTag_WAVE_5_0_B:				return "WAVE 5.0 B";
		case kAudioChannelLayoutTag_WAVE_5_1_B:				return "WAVE 5.1 B";
		case kAudioChannelLayoutTag_WAVE_6_1:				return "WAVE 6.1";
		case kAudioChannelLayoutTag_WAVE_7_1:				return "WAVE 7.1";
		case kAudioChannelLayoutTag_Atmos_5_1_2:			return "Atmos 5.1.2";
		case kAudioChannelLayoutTag_Atmos_5_1_4:			return "Atmos 5.1.4";
		case kAudioChannelLayoutTag_Atmos_7_1_2:			return "Atmos 7.1.2";
		case kAudioChannelLayoutTag_Atmos_7_1_4:			return "Atmos 7.1.4";
		case kAudioChannelLayoutTag_Atmos_9_1_6:			return "Atmos 9.1.6";
		case kAudioChannelLayoutTag_Logic_4_0_C:			return "Logic 4.0 C";
		case kAudioChannelLayoutTag_Logic_6_0_B:			return "Logic 6.0 B";
		case kAudioChannelLayoutTag_Logic_6_1_B:			return "Logic 6.1 B";
		case kAudioChannelLayoutTag_Logic_6_1_D:			return "Logic 6.1 D";
		case kAudioChannelLayoutTag_Logic_7_1_B:			return "Logic 7.1 B";
		case kAudioChannelLayoutTag_Logic_Atmos_7_1_4_B:	return "Logic Atmos 7.1.4 B";
		case kAudioChannelLayoutTag_Logic_Atmos_7_1_6:		return "Logic Atmos 7.1.6";
		case kAudioChannelLayoutTag_CICP_13:				return "CICP 13";
		case kAudioChannelLayoutTag_CICP_14:				return "CICP 14";
		case kAudioChannelLayoutTag_CICP_15:				return "CICP 15";
		case kAudioChannelLayoutTag_CICP_16:				return "CICP 16";
		case kAudioChannelLayoutTag_CICP_17:				return "CICP 17";
		case kAudioChannelLayoutTag_CICP_18:				return "CICP 18";
		case kAudioChannelLayoutTag_CICP_19:				return "CICP 19";
		case kAudioChannelLayoutTag_CICP_20:				return "CICP 20";
		case kAudioChannelLayoutTag_Ogg_5_0:				return "Ogg 5.0";
		case kAudioChannelLayoutTag_Ogg_5_1:				return "Ogg 5.1";
		case kAudioChannelLayoutTag_Ogg_6_1:				return "Ogg 6.1";
		case kAudioChannelLayoutTag_Ogg_7_1:				return "Ogg 7.1";
		case kAudioChannelLayoutTag_MPEG_5_0_E:				return "MPEG 5.0 E";
		case kAudioChannelLayoutTag_MPEG_5_1_E:				return "MPEG 5.1 E";
		case kAudioChannelLayoutTag_MPEG_6_1_B:				return "MPEG 6.1 B";
		case kAudioChannelLayoutTag_MPEG_7_1_D:				return "MPEG 7.1 D";

		case kAudioChannelLayoutTag_BeginReserved...kAudioChannelLayoutTag_EndReserved:
			return "Reserved";

		default:
			break;
	}

	switch(layoutTag & 0xFFFF0000) {
		case kAudioChannelLayoutTag_HOA_ACN_SN3D:			return "HOA ACN SN3D";
		case kAudioChannelLayoutTag_HOA_ACN_N3D:			return "HOA ACN N3D";
		case kAudioChannelLayoutTag_DiscreteInOrder:		return "Discrete in Order";
		case kAudioChannelLayoutTag_Unknown:				return "Unknown";

		default:
			break;
	}

	return nullptr;
}

/// A std::unique_ptr deleter for CFTypeRef objects.
struct cf_type_ref_deleter {
	void operator()(CFTypeRef cf) { CFRelease(cf); }
};

/// A std::unique_ptr holding a CFTypeRef object.
template <typename T>
using cf_type_ref_unique_ptr = std::unique_ptr<std::remove_pointer_t<T>, cf_type_ref_deleter>;

/// A std::unique_ptr holding a CFStringRef.
using cf_string_unique_ptr = cf_type_ref_unique_ptr<CFStringRef>;

/// Returns the name of the channel for an AudioChannelLabel.
cf_string_unique_ptr CopyChannelLabelName(AudioChannelLabel channelLabel, bool shortName) noexcept
{
	const auto property = shortName ? kAudioFormatProperty_ChannelShortName : kAudioFormatProperty_ChannelName;
	CFStringRef channelName = nullptr;
	UInt32 dataSize = sizeof(channelName);
	OSStatus result = AudioFormatGetProperty(property, sizeof(channelLabel), &channelLabel, &dataSize, &channelName);
	if(result != noErr)
		return nullptr;
	return cf_string_unique_ptr{channelName};
}

/// Joins strings from array separated by delimiter.
cf_string_unique_ptr JoinStringArray(CFArrayRef array, CFStringRef delimiter) noexcept
{
	if(!array)
		return nullptr;

	cf_type_ref_unique_ptr<CFMutableStringRef> result{CFStringCreateMutable(kCFAllocatorDefault, 0)};

	const CFIndex count = CFArrayGetCount(array);
	for(CFIndex i = 0; i < count; ++i) {
		auto value = CFArrayGetValueAtIndex(array, i);
		// Skip non-strings
		if(CFGetTypeID(value) != CFStringGetTypeID())
			continue;
		CFStringAppend(result.get(), static_cast<CFStringRef>(value));
		if(delimiter && i < count - 1)
			CFStringAppend(result.get(), delimiter);
	}

	return result;
}

} /* namespace */

// MARK: AudioChannelLayout Helper Functions

AudioChannelLayout * CoreAudio::AllocateAudioChannelLayout(UInt32 numberChannelDescriptions) noexcept
{
	const auto layoutSize = AudioChannelLayoutSize(numberChannelDescriptions);
	auto channelLayout = static_cast<AudioChannelLayout *>(std::malloc(layoutSize));
	if(!channelLayout)
		return nullptr;
	std::memset(channelLayout, 0, layoutSize);
	channelLayout->mNumberChannelDescriptions = numberChannelDescriptions;
	return channelLayout;
}

AudioChannelLayout * CoreAudio::CopyAudioChannelLayout(const AudioChannelLayout *other) noexcept
{
	if(!other)
		return nullptr;
	auto channelLayout = AllocateAudioChannelLayout(other->mNumberChannelDescriptions);
	if(channelLayout)
		std::memcpy(channelLayout, other, AudioChannelLayoutSize(other->mNumberChannelDescriptions));
	return channelLayout;
}

bool CoreAudio::AudioChannelLayoutsAreEqual(const AudioChannelLayout * _Nullable lhs, const AudioChannelLayout * _Nullable rhs) noexcept
{
	if(!lhs && !rhs)
		return true;
	else if((lhs && !rhs) || (!lhs && rhs))
		return false;

	const auto lsize = AudioChannelLayoutSize(lhs->mNumberChannelDescriptions);
	const auto rsize = AudioChannelLayoutSize(rhs->mNumberChannelDescriptions);
	if(lsize != rsize)
		return false;
	return !memcmp(lhs, rhs, lsize);
}

bool CoreAudio::AudioChannelLayoutsAreEquivalent(const AudioChannelLayout *lhs, const AudioChannelLayout *rhs) noexcept
{
	if(!lhs && !rhs)
		return true;
	else if(lhs && !rhs) {
		const auto tag = lhs->mChannelLayoutTag;
		if(tag == kAudioChannelLayoutTag_Mono || tag == kAudioChannelLayoutTag_Stereo)
			return true;
	}
	else if(!lhs && rhs) {
		const auto tag = rhs->mChannelLayoutTag;
		if(tag == kAudioChannelLayoutTag_Mono || tag == kAudioChannelLayoutTag_Stereo)
			return true;
	}

	if(!lhs || !rhs)
		return false;

	const AudioChannelLayout *layouts [] = { lhs, rhs, };
	UInt32 layoutsEquivalent = 0;
	UInt32 propertySize = sizeof(layoutsEquivalent);
	OSStatus result = AudioFormatGetProperty(kAudioFormatProperty_AreChannelLayoutsEquivalent, sizeof(layouts), static_cast<const void *>(layouts), &propertySize, &layoutsEquivalent);
	if(result != noErr)
		return false;
	return layoutsEquivalent;
}

CFStringRef CoreAudio::CopyAudioChannelLayoutName(const AudioChannelLayout * channelLayout, bool simpleName) noexcept
{
	if(!channelLayout)
		return nullptr;
	CFStringRef name = nullptr;
	UInt32 dataSize = sizeof(name);
	OSStatus result = AudioFormatGetProperty(simpleName ? kAudioFormatProperty_ChannelLayoutSimpleName : kAudioFormatProperty_ChannelLayoutName, static_cast<UInt32>(AudioChannelLayoutSize(channelLayout)), channelLayout, &dataSize, &name);
	if(result != noErr)
		return nullptr;
	return name;
}

CFStringRef CoreAudio::CopyAudioChannelLayoutDescription(const AudioChannelLayout *channelLayout) noexcept
{
	if(!channelLayout)
		return nullptr;

	CFMutableStringRef result = CFStringCreateMutable(kCFAllocatorDefault, 0);
	if(!result)
		return nullptr;

	cf_string_unique_ptr layoutName{CopyAudioChannelLayoutName(channelLayout)};

	if(channelLayout->mChannelLayoutTag == kAudioChannelLayoutTag_UseChannelDescriptions) {
		// kAudioFormatProperty_ChannelLayoutName returns '!fmt' for kAudioChannelLabel_UseCoordinates
		if(layoutName) {
			CFStringAppendFormat(result, nullptr, CFSTR("%u channel descriptions, %@"), channelLayout->mNumberChannelDescriptions, layoutName.get());
			return result;
		}

		CFStringAppendFormat(result, nullptr, CFSTR("%u channel descriptions"), channelLayout->mNumberChannelDescriptions);

		cf_type_ref_unique_ptr<CFMutableArrayRef> array{CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks)};

		const AudioChannelDescription *desc = channelLayout->mChannelDescriptions;
		for(UInt32 i = 0; i < channelLayout->mNumberChannelDescriptions; ++i, ++desc) {
			if(desc->mChannelLabel == kAudioChannelLabel_UseCoordinates) {
				cf_string_unique_ptr coordinateString{};
				if(desc->mChannelFlags & kAudioChannelFlags_RectangularCoordinates)
					coordinateString.reset(CFStringCreateWithFormat(kCFAllocatorDefault, nullptr, CFSTR("[x: %g, y: %g, z: %g%s]"), desc->mCoordinates[0], desc->mCoordinates[1], desc->mCoordinates[2], desc->mChannelFlags & kAudioChannelFlags_Meters ? " m" : ""));
				else if(desc->mChannelFlags & kAudioChannelFlags_SphericalCoordinates)
					coordinateString.reset(CFStringCreateWithFormat(kCFAllocatorDefault, nullptr, CFSTR("[r: %g, θ: %g, φ: %g%s]"), desc->mCoordinates[2], desc->mCoordinates[1], desc->mCoordinates[0], desc->mChannelFlags & kAudioChannelFlags_Meters ? " m" : ""));
				else
					coordinateString.reset(CFStringCreateWithFormat(kCFAllocatorDefault, nullptr, CFSTR("[?! %g, %g, %g%s]"), desc->mCoordinates[0], desc->mCoordinates[1], desc->mCoordinates[2], desc->mChannelFlags & kAudioChannelFlags_Meters ? " m" : ""));

				CFArrayAppendValue(array.get(), reinterpret_cast<const void *>(coordinateString.get()));
			}
			else {
				if(const auto channelName = CopyChannelLabelName(desc->mChannelLabel, true); channelName)
					CFArrayAppendValue(array.get(), reinterpret_cast<const void *>(channelName.get()));
				else
					CFArrayAppendValue(array.get(), CFSTR("?"));
			}
		}

		auto channelNamesString = JoinStringArray(array.get(), CFSTR(" "));
		CFStringAppendFormat(result, nullptr, CFSTR(", %@"), channelNamesString.get());
	}
	else if(channelLayout->mChannelLayoutTag == kAudioChannelLayoutTag_UseChannelBitmap) {
		CFStringAppendFormat(result, nullptr, CFSTR("Bitmap %#x (%u ch)"), channelLayout->mChannelBitmap, __builtin_popcount(channelLayout->mChannelBitmap));
		if(layoutName)
			CFStringAppendFormat(result, nullptr, CFSTR(", %@"), layoutName.get());
	}
	else {
		CFStringAppendFormat(result, nullptr, CFSTR("%s (0x%x, %u ch)"), GetChannelLayoutTagName(channelLayout->mChannelLayoutTag), channelLayout->mChannelLayoutTag, channelLayout->mChannelLayoutTag & 0xffff);
		if(layoutName)
			CFStringAppendFormat(result, nullptr, CFSTR(", %@"), layoutName.get());
	}

	return result;
}

// Constants
const CoreAudio::CAChannelLayout CoreAudio::CAChannelLayout::Mono		= CAChannelLayout(kAudioChannelLayoutTag_Mono);
const CoreAudio::CAChannelLayout CoreAudio::CAChannelLayout::Stereo 	= CAChannelLayout(kAudioChannelLayoutTag_Stereo);

CoreAudio::CAChannelLayout CoreAudio::CAChannelLayout::ChannelLayoutWithBitmap(AudioChannelBitmap channelBitmap)
{
	CAChannelLayout channelLayout{kAudioChannelLayoutTag_UseChannelBitmap};
	channelLayout.channelLayout_->mChannelBitmap = channelBitmap;

	// Attempt to convert the bitmap to a layout tag
	AudioChannelLayoutTag tag;
	UInt32 dataSize = sizeof(tag);
	OSStatus result = AudioFormatGetProperty(kAudioFormatProperty_TagForChannelLayout, static_cast<UInt32>(AudioChannelLayoutSize(channelLayout.channelLayout_)), channelLayout.channelLayout_, &dataSize, &tag);
	if(result == noErr) {
		channelLayout.channelLayout_->mChannelLayoutTag = tag;
		channelLayout.channelLayout_->mChannelBitmap = 0;
	}

	return channelLayout;
}

CoreAudio::CAChannelLayout CoreAudio::CAChannelLayout::ChannelLayoutWithTag(AudioChannelLayoutTag layoutTag)
{
	return CAChannelLayout{layoutTag};
}

CoreAudio::CAChannelLayout CoreAudio::CAChannelLayout::ChannelLayoutWithChannelLabels(std::vector<AudioChannelLabel> channelLabels)
{
	return CAChannelLayout{channelLabels};
}

CoreAudio::CAChannelLayout::CAChannelLayout(const CAChannelLayout& other)
: CAChannelLayout{}
{
	*this = other;
}

CoreAudio::CAChannelLayout& CoreAudio::CAChannelLayout::operator=(const CAChannelLayout& other)
{
	if(this == &other)
		return *this;
	if(channelLayout_ && other.channelLayout_ && channelLayout_->mNumberChannelDescriptions == other.channelLayout_->mNumberChannelDescriptions)
		memcpy(channelLayout_, other.channelLayout_, AudioChannelLayoutSize(other.channelLayout_->mNumberChannelDescriptions));
	else {
		const auto channelLayout = CopyAudioChannelLayout(other.channelLayout_);
		if(other.channelLayout_ && !channelLayout)
			throw std::bad_alloc();
		Reset(channelLayout);
	}
	return *this;
}

CoreAudio::CAChannelLayout::CAChannelLayout(AudioChannelLayoutTag layoutTag)
: channelLayout_{AllocateAudioChannelLayout(0)}
{
	if(!channelLayout_)
		throw std::bad_alloc();
	channelLayout_->mChannelLayoutTag = layoutTag;
}

CoreAudio::CAChannelLayout::CAChannelLayout(std::vector<AudioChannelLabel> channelLabels)
: channelLayout_{AllocateAudioChannelLayout(static_cast<UInt32>(channelLabels.size()))}
{
	if(!channelLayout_)
		throw std::bad_alloc();

	channelLayout_->mChannelLayoutTag = kAudioChannelLayoutTag_UseChannelDescriptions;
	for(std::vector<AudioChannelLabel>::size_type i = 0; i != channelLabels.size(); ++i)
		channelLayout_->mChannelDescriptions[i].mChannelLabel = channelLabels[i];

	// Attempt to convert the channel labels to a layout tag
	AudioChannelLayoutTag tag;
	UInt32 dataSize = sizeof(tag);
	OSStatus result = AudioFormatGetProperty(kAudioFormatProperty_TagForChannelLayout, static_cast<UInt32>(AudioChannelLayoutSize(channelLayout_)), channelLayout_, &dataSize, &tag);
	if(result == noErr) {
		const auto channelLayout = AllocateAudioChannelLayout(0);
		if(!channelLayout)
			throw std::bad_alloc();
		Reset(channelLayout);
		channelLayout_->mChannelLayoutTag = tag;
	}
}

CoreAudio::CAChannelLayout::CAChannelLayout(const AudioChannelLayout *other)
: channelLayout_{CopyAudioChannelLayout(other)}
{
	if(other && !channelLayout_)
		throw std::bad_alloc();
}

CoreAudio::CAChannelLayout& CoreAudio::CAChannelLayout::operator=(const AudioChannelLayout *other)
{
	const auto channelLayout = CopyAudioChannelLayout(other);
	if(other && !channelLayout)
		throw std::bad_alloc();
	Reset(channelLayout);
	return *this;
}

CoreAudio::CAChannelLayout::CAChannelLayout(CAChannelLayout&& other) noexcept
: channelLayout_{other.Release()}
{}

CoreAudio::CAChannelLayout& CoreAudio::CAChannelLayout::operator=(CAChannelLayout&& other) noexcept
{
	if(this != &other)
		Reset(other.Release());
	return *this;
}

CoreAudio::CAChannelLayout::~CAChannelLayout() noexcept
{
	std::free(channelLayout_);
}

// MARK: Functionality

UInt32 CoreAudio::CAChannelLayout::ChannelCount() const noexcept
{
	if(!channelLayout_)
		return 0;
	else if(channelLayout_->mChannelLayoutTag == kAudioChannelLayoutTag_UseChannelDescriptions)
		return channelLayout_->mNumberChannelDescriptions;
	else if(channelLayout_->mChannelLayoutTag == kAudioChannelLayoutTag_UseChannelBitmap)
		return static_cast<UInt32>(__builtin_popcount(channelLayout_->mChannelBitmap));
	return AudioChannelLayoutTag_GetNumberOfChannels(channelLayout_->mChannelLayoutTag);
}

bool CoreAudio::CAChannelLayout::MapToLayout(const CAChannelLayout& outputLayout, std::vector<SInt32>& channelMap) const
{
	// No valid map exists for empty/unknown layouts
	if(!channelLayout_ || !outputLayout.channelLayout_)
		return false;

	const AudioChannelLayout *layouts [] = {
		channelLayout_,
		outputLayout.channelLayout_
	};

	const auto outputChannelCount = outputLayout.ChannelCount();
	if(outputChannelCount == 0)
		return false;

	channelMap.resize(outputChannelCount);
	UInt32 propertySize = static_cast<UInt32>(sizeof(SInt32) * channelMap.size());
	OSStatus result = AudioFormatGetProperty(kAudioFormatProperty_ChannelMap, sizeof(layouts), static_cast<const void *>(layouts), &propertySize, channelMap.data());
	return result == noErr;
}
