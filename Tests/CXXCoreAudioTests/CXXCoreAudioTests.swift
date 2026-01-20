//
// SPDX-FileCopyrightText: 2025 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXCoreAudio
//

import Testing
@testable import CXXCoreAudio

@Suite struct CXXCoreAudioTests {
    @Test func timeStamp() async {
        let ts = CXXCoreAudio.CATimeStamp(22050.0)
        #expect(ts.isValid())
        #expect(ts.sampleTimeIsValid())
        #expect(!ts.hostTimeIsValid())
    }

    @Test func valueRange() async {
        let vr = CXXCoreAudio.CAValueRange()
        #expect(vr.isValid())
        #expect(vr.contains(0))
        #expect(!vr.contains(1))
    }

    @Test func streamDescription() async {
        let fmt = CXXCoreAudio.CAStreamDescription(.float32, 44100, 2, false)
        #expect(fmt.IsPCM() == true)
        #expect(fmt.IsFloat() == true)
        #expect(fmt.IsInteger() == false)
        #expect(fmt.mSampleRate == 44100)
        #expect(fmt.ChannelCount() == 2)
        #expect(fmt.IsInterleaved() == false)
        #expect(fmt.IsNonInterleaved() == true)
    }

    @Test func channelLayout() async {
        let empty = CXXCoreAudio.CAChannelLayout()
        #expect(!empty.__convertToBool())
        #expect(empty.Size() == 0)
        #expect(empty.ChannelCount() == 0)
        let stereo = CXXCoreAudio.CAChannelLayout.Stereo
        #expect(stereo.ChannelCount() == 2)
    }

    @Test func audioBuffer() async {
        let empty = CXXCoreAudio.CAAudioBuffer()
        #expect(empty.FrameLength() == 0)
        #expect(empty.FrameCapacity() == 0)
    }

    @Test func ringBuffer() async {
        let empty = CXXCoreAudio.AudioRingBuffer()
        #expect(empty.__convertToBool() == false)
        #expect(empty.capacity() == 0)
        #expect(empty.availableFrames() == 0)
        #expect(empty.freeSpace() == empty.capacity())

        var rb = CXXCoreAudio.AudioRingBuffer()
        let std2ch = AudioStreamBasicDescription(mSampleRate: 44100, mFormatID: kAudioFormatLinearPCM, mFormatFlags: kAudioFormatFlagsNativeFloatPacked|kAudioFormatFlagIsNonInterleaved, mBytesPerPacket: 8, mFramesPerPacket: 8, mBytesPerFrame: 8, mChannelsPerFrame: 2, mBitsPerChannel: 32, mReserved: 0)
        #expect(rb.allocate(std2ch, 512) == true)
        #expect(rb.__convertToBool() == true)
        #expect(rb.capacity() == 512)
        #expect(rb.availableFrames() == 0)
        #expect(rb.freeSpace() == rb.capacity())

        rb.deallocate()
        #expect(rb.__convertToBool() == false)
        #expect(rb.capacity() == 0)
        #expect(rb.availableFrames() == 0)
        #expect(rb.freeSpace() == rb.capacity())
    }

    @Test func tsRingBuffer() async {
        let empty = CXXCoreAudio.CARingBuffer()
        #expect(empty.Capacity() == 0)
        #expect(empty.UnusedSpace() == empty.Capacity())
        var start: Int64 = 0, end: Int64 = 0
        #expect(empty.GetTimeBounds(&start, &end) == true)
        #expect(start == 0)
        #expect(end == 0)

        var rb = CXXCoreAudio.CARingBuffer()
        let std2ch = AudioStreamBasicDescription(mSampleRate: 44100, mFormatID: kAudioFormatLinearPCM, mFormatFlags: kAudioFormatFlagsNativeFloatPacked|kAudioFormatFlagIsNonInterleaved, mBytesPerPacket: 8, mFramesPerPacket: 8, mBytesPerFrame: 8, mChannelsPerFrame: 2, mBitsPerChannel: 32, mReserved: 0)
        #expect(rb.Allocate(std2ch, 512) == true)
        #expect(rb.Capacity() == 511)
        #expect(rb.UnusedSpace() == rb.Capacity())

        let mono8bit = AudioStreamBasicDescription(mSampleRate: 22050, mFormatID: kAudioFormatLinearPCM, mFormatFlags: kAudioFormatFlagIsPacked|kAudioFormatFlagsNativeEndian|kAudioFormatFlagIsNonInterleaved, mBytesPerPacket: 1, mFramesPerPacket: 1, mBytesPerFrame: 1, mChannelsPerFrame: 1, mBitsPerChannel: 8, mReserved: 0)

        let input_abl = AudioBufferList.allocate(maximumBuffers: 1)
        input_abl[0] = AudioBuffer(mNumberChannels: 1, mDataByteSize: 128, mData: malloc(128))
        defer {
            for buffer in input_abl {
                free(buffer.mData)
            }
            free(input_abl.unsafeMutablePointer)
        }

        let output_abl = AudioBufferList.allocate(maximumBuffers: 1)
        output_abl[0] = AudioBuffer(mNumberChannels: 1, mDataByteSize: 128, mData: malloc(128))
        defer {
            for buffer in output_abl {
                free(buffer.mData)
            }
            free(output_abl.unsafeMutablePointer)
        }

        memset(input_abl[0].mData, 0xA, 32)
        memset(input_abl[0].mData?.advanced(by: 32), 0xB, 32)
        memset(input_abl[0].mData?.advanced(by: 64), 0xC, 32)
        memset(input_abl[0].mData?.advanced(by: 96), 0xD, 32)

        memset(output_abl[0].mData, 0, 128)

        #expect(rb.Allocate(mono8bit, 256) == true)

        #expect(rb.Write(input_abl.unsafePointer, 128, 0) == true)
        #expect(rb.GetTimeBounds(&start, &end) == true)
        #expect(start == 0)
        #expect(end == 128)

        #expect(rb.Read(output_abl.unsafeMutablePointer, 128, 0) == true)
        #expect(output_abl[0].mDataByteSize == 128)
        #expect(memcmp(input_abl[0].mData, output_abl[0].mData, 128) == 0)

        #expect(rb.Read(output_abl.unsafeMutablePointer, 128, 64) == true)
        #expect(output_abl[0].mDataByteSize == 64)

        #expect(memcmp(input_abl[0].mData?.advanced(by: 64), output_abl[0].mData, 64) == 0)
    }
}
