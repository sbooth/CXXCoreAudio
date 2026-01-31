//
// SPDX-FileCopyrightText: 2014 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXCoreAudio
//

#include "core_audio/StreamDescription.hpp"

#include <AudioToolbox/AudioFormat.h>

#include <libkern/OSByteOrder.h>

namespace {

/// Returns a descriptive format name for formatID or nullptr if unknown.
CFStringRef _Nullable getFormatIDName(AudioFormatID formatID) noexcept {
    switch (formatID) {
    case kAudioFormatLinearPCM:
        return CFSTR("Linear PCM");
    case kAudioFormatAC3:
        return CFSTR("AC-3");
    case kAudioFormat60958AC3:
        return CFSTR("AC-3 over IEC 60958");
    case kAudioFormatAppleIMA4:
        return CFSTR("IMA 4:1 ADPCM");
    case kAudioFormatMPEG4AAC:
        return CFSTR("MPEG-4 Low Complexity AAC");
    case kAudioFormatMPEG4CELP:
        return CFSTR("MPEG-4 CELP");
    case kAudioFormatMPEG4HVXC:
        return CFSTR("MPEG-4 HVXC");
    case kAudioFormatMPEG4TwinVQ:
        return CFSTR("MPEG-4 TwinVQ");
    case kAudioFormatMACE3:
        return CFSTR("MACE 3:1");
    case kAudioFormatMACE6:
        return CFSTR("MACE 6:1");
    case kAudioFormatULaw:
        return CFSTR("µ-law 2:1");
    case kAudioFormatALaw:
        return CFSTR("A-law 2:1");
    case kAudioFormatQDesign:
        return CFSTR("QDesign music");
    case kAudioFormatQDesign2:
        return CFSTR("QDesign2 music");
    case kAudioFormatQUALCOMM:
        return CFSTR("QUALCOMM PureVoice");
    case kAudioFormatMPEGLayer1:
        return CFSTR("MPEG-1/2 Layer I");
    case kAudioFormatMPEGLayer2:
        return CFSTR("MPEG-1/2 Layer II");
    case kAudioFormatMPEGLayer3:
        return CFSTR("MPEG-1/2 Layer III");
    case kAudioFormatTimeCode:
        return CFSTR("Stream of IOAudioTimeStamps");
    case kAudioFormatMIDIStream:
        return CFSTR("Stream of MIDIPacketLists");
    case kAudioFormatParameterValueStream:
        return CFSTR("Float32 side-chain");
    case kAudioFormatAppleLossless:
        return CFSTR("Apple Lossless");
    case kAudioFormatMPEG4AAC_HE:
        return CFSTR("MPEG-4 High Efficiency AAC");
    case kAudioFormatMPEG4AAC_LD:
        return CFSTR("MPEG-4 AAC Low Delay");
    case kAudioFormatMPEG4AAC_ELD:
        return CFSTR("MPEG-4 AAC Enhanced Low Delay");
    case kAudioFormatMPEG4AAC_ELD_SBR:
        return CFSTR("MPEG-4 AAC Enhanced Low Delay with SBR extension");
    case kAudioFormatMPEG4AAC_ELD_V2:
        return CFSTR("MPEG-4 AAC Enhanced Low Delay Version 2");
    case kAudioFormatMPEG4AAC_HE_V2:
        return CFSTR("MPEG-4 High Efficiency AAC Version 2");
    case kAudioFormatMPEG4AAC_Spatial:
        return CFSTR("MPEG-4 Spatial Audio");
    case kAudioFormatMPEGD_USAC:
        return CFSTR("MPEG-D Unified Speech and Audio Coding");
    case kAudioFormatAMR:
        return CFSTR("AMR Narrow Band");
    case kAudioFormatAMR_WB:
        return CFSTR("AMR Wide Band");
    case kAudioFormatAudible:
        return CFSTR("Audible");
    case kAudioFormatiLBC:
        return CFSTR("iLBC narrow band");
    case kAudioFormatDVIIntelIMA:
        return CFSTR("DVI/Intel IMA ADPCM");
    case kAudioFormatMicrosoftGSM:
        return CFSTR("Microsoft GSM 6.10");
    case kAudioFormatAES3:
        return CFSTR("AES3-2003");
    case kAudioFormatEnhancedAC3:
        return CFSTR("Enhanced AC-3");
    case kAudioFormatFLAC:
        return CFSTR("Free Lossless Audio Codec");
    case kAudioFormatOpus:
        return CFSTR("Opus");
    case kAudioFormatAPAC:
        return CFSTR("Apple Positional Audio Codec");
    default:
        return nullptr;
    }
}

/// Returns true if c is a printable ASCII character.
constexpr bool isPrintableASCII(unsigned char c) noexcept { return c > 0x1f && c < 0x7f; }

/// Creates a string representation of a four-character code.
CFStringRef _Nullable createFourCharCodeString(UInt32 fourcc) noexcept CF_RETURNS_RETAINED {
    union {
        UInt32 ui32;
        unsigned char str[4];
    } u;
    u.ui32 = OSSwapHostToBigInt32(fourcc);

    if (isPrintableASCII(u.str[0]) && isPrintableASCII(u.str[1]) && isPrintableASCII(u.str[2]) &&
        isPrintableASCII(u.str[3])) {
        return CFStringCreateWithFormat(kCFAllocatorDefault, nullptr, CFSTR("'%.4s'"), u.str);
    } else {
        return CFStringCreateWithFormat(kCFAllocatorDefault, nullptr, CFSTR("0x%.02x%.02x%.02x%.02x"), u.str[0],
                                        u.str[1], u.str[2], u.str[3]);
    }
}

} /* namespace */

std::optional<core_audio::CommonPCMFormat>
core_audio::identifyCommonPCMFormat(const AudioStreamBasicDescription &streamDescription) noexcept {
    if (streamDescription.mFramesPerPacket != 1 ||
        streamDescription.mBytesPerFrame != streamDescription.mBytesPerPacket ||
        streamDescription.mChannelsPerFrame == 0) {
        return std::nullopt;
    }

    // Exclude non-PCM, non-native endian, non-implicitly packed formats
    if (streamDescription.mFormatID != kAudioFormatLinearPCM ||
        (streamDescription.mFormatFlags & kAudioFormatFlagIsBigEndian) != kAudioFormatFlagsNativeEndian ||
        ((streamDescription.mBitsPerChannel / 8) *
         (((streamDescription.mFormatFlags & kAudioFormatFlagIsNonInterleaved) == 0)
                  ? streamDescription.mChannelsPerFrame
                  : 1)) != streamDescription.mBytesPerFrame) {
        return std::nullopt;
    }

    if ((streamDescription.mFormatFlags & kAudioFormatFlagIsSignedInteger) == kAudioFormatFlagIsSignedInteger) {
        // Disqualify fixed point
        if ((streamDescription.mFormatFlags & kAudioFormatFlagIsFloat) == 0 &&
            ((streamDescription.mFormatFlags & kLinearPCMFormatFlagsSampleFractionMask) >>
             kLinearPCMFormatFlagsSampleFractionShift) > 0) {
            return std::nullopt;
        }

        if (streamDescription.mBitsPerChannel == 16) {
            return CommonPCMFormat::int16;
        }
        if (streamDescription.mBitsPerChannel == 32) {
            return CommonPCMFormat::int32;
        }
    } else if ((streamDescription.mFormatFlags & kAudioFormatFlagIsFloat) == kAudioFormatFlagIsFloat) {
        if (streamDescription.mBitsPerChannel == 32) {
            return CommonPCMFormat::float32;
        }
        if (streamDescription.mBitsPerChannel == 64) {
            return CommonPCMFormat::float64;
        }
    }

    return std::nullopt;
}

cf::CFString
core_audio::copyAudioStreamBasicDescriptionFormatName(const AudioStreamBasicDescription &streamDescription) noexcept {
    CFStringRef name = nullptr;
    UInt32 dataSize = sizeof name;
    OSStatus result = AudioFormatGetProperty(kAudioFormatProperty_FormatName, sizeof streamDescription,
                                             &streamDescription, &dataSize, &name);
    if (result != noErr) {
        return nullptr;
    }
    return cf::CFString(name);
}

cf::CFString core_audio::copyAudioStreamBasicDescriptionFormatDescription(
        const AudioStreamBasicDescription &streamDescription) noexcept {
    cf::CFMutableString result{CFStringCreateMutable(kCFAllocatorDefault, 0)};
    if (!result) {
        return nullptr;
    }

    // Channels and sample rate
    CFStringAppendFormat(result, nullptr, CFSTR("%u ch @ %g Hz, "), streamDescription.mChannelsPerFrame,
                         streamDescription.mSampleRate);

    // Shorter description for common formats
    if (const auto commonPCMFormat = identifyCommonPCMFormat(streamDescription); commonPCMFormat.has_value()) {
        switch (commonPCMFormat.value()) {
        case CommonPCMFormat::int16:
            CFStringAppendCString(result, "Int16, ", kCFStringEncodingASCII);
            break;
        case CommonPCMFormat::int32:
            CFStringAppendCString(result, "Int32, ", kCFStringEncodingASCII);
            break;
        case CommonPCMFormat::float32:
            CFStringAppendCString(result, "Float32, ", kCFStringEncodingASCII);
            break;
        case CommonPCMFormat::float64:
            CFStringAppendCString(result, "Float64, ", kCFStringEncodingASCII);
            break;
        }

        if ((streamDescription.mFormatFlags & kAudioFormatFlagIsNonInterleaved) == kAudioFormatFlagIsNonInterleaved) {
            CFStringAppendCString(result, "deinterleaved", kCFStringEncodingASCII);
        } else {
            CFStringAppendCString(result, "interleaved", kCFStringEncodingASCII);
        }

        return cf::CFString(result.leak());
    }

    if (streamDescription.mFormatID == kAudioFormatLinearPCM) {
        // Bit depth
        const auto fractionalBits = (streamDescription.mFormatFlags & kLinearPCMFormatFlagsSampleFractionMask) >>
                                    kLinearPCMFormatFlagsSampleFractionShift;
        if (fractionalBits > 0) {
            CFStringAppendFormat(result, nullptr, CFSTR("%d.%d-bit"),
                                 streamDescription.mBitsPerChannel - fractionalBits, fractionalBits);
        } else {
            CFStringAppendFormat(result, nullptr, CFSTR("%d-bit"), streamDescription.mBitsPerChannel);
        }

        const auto interleavedChannelCount = ((streamDescription.mFormatFlags & kAudioFormatFlagIsNonInterleaved) == 0)
                                                     ? streamDescription.mChannelsPerFrame
                                                     : 1;
        const auto sampleWordSize =
                (interleavedChannelCount == 0 || streamDescription.mBytesPerFrame % interleavedChannelCount != 0)
                        ? 0
                        : streamDescription.mBytesPerFrame / interleavedChannelCount;

        // Endianness
        if (sampleWordSize > 1) {
            CFStringAppendCString(result,
                                  (streamDescription.mFormatFlags & kAudioFormatFlagIsBigEndian) ==
                                                  kAudioFormatFlagIsBigEndian
                                          ? " big-endian"
                                          : " little-endian",
                                  kCFStringEncodingASCII);
        }

        // Sign
        auto isInteger = (streamDescription.mFormatFlags & kAudioFormatFlagIsFloat) == 0;
        if (isInteger) {
            CFStringAppendCString(result,
                                  (streamDescription.mFormatFlags & kAudioFormatFlagIsSignedInteger) ==
                                                  kAudioFormatFlagIsSignedInteger
                                          ? " signed"
                                          : " unsigned",
                                  kCFStringEncodingASCII);
        }

        // Integer or floating
        CFStringAppendCString(result, isInteger ? " integer" : " float", kCFStringEncodingASCII);

        // Packedness and alignment
        if (sampleWordSize > 0) {
            // Implicitly packed
            if (((streamDescription.mBitsPerChannel / 8) * interleavedChannelCount) ==
                streamDescription.mBytesPerFrame) {
                CFStringAppendCString(result, ", packed", kCFStringEncodingASCII);
            }
            // Unaligned
            else if ((sampleWordSize << 3) != streamDescription.mBitsPerChannel ||
                     (streamDescription.mBitsPerChannel & 7) != 0) {
                CFStringAppendCString(result,
                                      (streamDescription.mFormatFlags & kAudioFormatFlagIsAlignedHigh) ==
                                                      kAudioFormatFlagIsAlignedHigh
                                              ? ", high-aligned"
                                              : ", low-aligned",
                                      kCFStringEncodingASCII);
            }

            CFStringAppendFormat(result, nullptr, CFSTR(" in %d bytes"), sampleWordSize);
        }

        if ((streamDescription.mFormatFlags & kAudioFormatFlagIsNonInterleaved) == kAudioFormatFlagIsNonInterleaved) {
            CFStringAppendCString(result, ", deinterleaved", kCFStringEncodingASCII);
        }
    } else if (streamDescription.mFormatID == kAudioFormatAppleLossless ||
               streamDescription.mFormatID == kAudioFormatFLAC) {
        if (CFStringRef formatIDString = getFormatIDName(streamDescription.mFormatID); formatIDString) {
            CFStringAppend(result, formatIDString);
        } else if (CFStringRef fourCC = createFourCharCodeString(streamDescription.mFormatID); fourCC) {
            CFStringAppend(result, fourCC);
            CFRelease(fourCC);
        } else {
            CFStringAppendFormat(result, nullptr, CFSTR("0x%.08x"), streamDescription.mFormatID);
        }

        CFStringAppendCString(result, ", ", kCFStringEncodingASCII);

        UInt32 sourceBitDepth = 0;
        switch (streamDescription.mFormatFlags) {
        case kAppleLosslessFormatFlag_16BitSourceData:
            sourceBitDepth = 16;
            break;
        case kAppleLosslessFormatFlag_20BitSourceData:
            sourceBitDepth = 20;
            break;
        case kAppleLosslessFormatFlag_24BitSourceData:
            sourceBitDepth = 24;
            break;
        case kAppleLosslessFormatFlag_32BitSourceData:
            sourceBitDepth = 32;
            break;
        }

        if (sourceBitDepth != 0) {
            CFStringAppendFormat(result, nullptr, CFSTR("from %d-bit source, "), sourceBitDepth);
        } else {
            CFStringAppendCString(result, "from UNKNOWN source bit depth, ", kCFStringEncodingASCII);
        }

        CFStringAppendFormat(result, nullptr, CFSTR("%d frames/packet"), streamDescription.mFramesPerPacket);
    } else {
        if (CFStringRef formatIDString = getFormatIDName(streamDescription.mFormatID); formatIDString) {
            CFStringAppend(result, formatIDString);
        } else if (CFStringRef fourCC = createFourCharCodeString(streamDescription.mFormatID); fourCC) {
            CFStringAppend(result, fourCC);
            CFRelease(fourCC);
        } else {
            CFStringAppendFormat(result, nullptr, CFSTR("0x%.08x"), streamDescription.mFormatID);
        }

        // Format flags
        if (streamDescription.mFormatFlags != 0) {
            CFStringAppendFormat(result, nullptr, CFSTR(" (%#x)"), streamDescription.mFormatFlags);
        }

        CFStringAppendFormat(result, nullptr,
                             CFSTR(", %u bits/channel, %u bytes/packet, %u frames/packet, %u bytes/frame"),
                             streamDescription.mBitsPerChannel, streamDescription.mBytesPerPacket,
                             streamDescription.mFramesPerPacket, streamDescription.mBytesPerFrame);
    }

    return cf::CFString(result.leak());
}

core_audio::StreamDescription::StreamDescription(CommonPCMFormat commonPCMFormat, Float64 sampleRate,
                                                 UInt32 channelsPerFrame, bool isInterleaved) noexcept
    : AudioStreamBasicDescription{} {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-anon-enum-enum-conversion"
    switch (commonPCMFormat) {
    case CommonPCMFormat::float32:
        FillOutASBDForLPCM(*this, sampleRate, channelsPerFrame, 32, 32, true,
                           kAudioFormatFlagIsBigEndian == kAudioFormatFlagsNativeEndian, !isInterleaved);
        break;
    case CommonPCMFormat::float64:
        FillOutASBDForLPCM(*this, sampleRate, channelsPerFrame, 64, 64, true,
                           kAudioFormatFlagIsBigEndian == kAudioFormatFlagsNativeEndian, !isInterleaved);
        break;
    case CommonPCMFormat::int16:
        FillOutASBDForLPCM(*this, sampleRate, channelsPerFrame, 16, 16, false,
                           kAudioFormatFlagIsBigEndian == kAudioFormatFlagsNativeEndian, !isInterleaved);
        break;
    case CommonPCMFormat::int32:
        FillOutASBDForLPCM(*this, sampleRate, channelsPerFrame, 32, 32, false,
                           kAudioFormatFlagIsBigEndian == kAudioFormatFlagsNativeEndian, !isInterleaved);
        break;
    }
#pragma clang diagnostic pop
}

bool core_audio::StreamDescription::getNonInterleavedEquivalent(AudioStreamBasicDescription &format) const noexcept {
    if (!isPCM() || mChannelsPerFrame == 0) {
        return false;
    }
    format = *this;
    if (isInterleaved()) {
        format.mFormatFlags |= kAudioFormatFlagIsNonInterleaved;
        format.mBytesPerPacket /= mChannelsPerFrame;
        format.mBytesPerFrame /= mChannelsPerFrame;
    }
    return true;
}

bool core_audio::StreamDescription::getInterleavedEquivalent(AudioStreamBasicDescription &format) const noexcept {
    if (!isPCM()) {
        return false;
    }
    format = *this;
    if (!isInterleaved()) {
        format.mFormatFlags &= ~kAudioFormatFlagIsNonInterleaved;
        format.mBytesPerPacket *= mChannelsPerFrame;
        format.mBytesPerFrame *= mChannelsPerFrame;
    }
    return true;
}

bool core_audio::StreamDescription::getStandardEquivalent(AudioStreamBasicDescription &format) const noexcept {
    if (!isPCM()) {
        return false;
    }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-anon-enum-enum-conversion"
    FillOutASBDForLPCM(format, mSampleRate, mChannelsPerFrame, 32, 32, true,
                       kAudioFormatFlagIsBigEndian == kAudioFormatFlagsNativeEndian, true);
#pragma clang diagnostic pop
    return true;
}
