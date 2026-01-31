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
        let ts = core_audio.TimeStamp(22050.0)
        #expect(ts.isValid())
        #expect(ts.sampleTimeIsValid())
        #expect(!ts.hostTimeIsValid())
    }

    @Test func valueRange() async {
        let vr = core_audio.ValueRange()
        #expect(vr.isValid())
        #expect(vr.contains(0))
        #expect(!vr.contains(1))
    }

    @Test func streamDescription() async {
        let fmt = core_audio.StreamDescription(.float32, 44100, 2, false)
        #expect(fmt.isPCM() == true)
        #expect(fmt.isFloat() == true)
        #expect(fmt.isInteger() == false)
        #expect(fmt.mSampleRate == 44100)
        #expect(fmt.channelCount() == 2)
        #expect(fmt.isInterleaved() == false)
        #expect(fmt.isNonInterleaved() == true)
    }

    @Test func channelLayout() async {
        let empty = core_audio.ChannelLayout()
        #expect(!empty.__convertToBool())
        #expect(empty.size() == 0)
        #expect(empty.channelCount() == 0)
        let stereo = core_audio.ChannelLayout.Stereo
        #expect(stereo.channelCount() == 2)
    }

    @Test func audioBuffer() async {
        let empty = core_audio.BufferList()
        #expect(empty.frameLength() == 0)
        #expect(empty.frameCapacity() == 0)
    }

    @Test func ringBuffer() async {
        let empty = core_audio.RingBuffer()
        #expect(empty.__convertToBool() == false)
        #expect(empty.capacity() == 0)
        #expect(empty.availableFrames() == 0)
        #expect(empty.freeSpace() == empty.capacity())

        var rb = core_audio.RingBuffer()
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
        let empty = core_audio.TemporalRingBuffer()
        #expect(empty.capacity() == 0)
        #expect(empty.unusedSpace() == empty.capacity())
        var start: Int64 = 0, end: Int64 = 0
        #expect(empty.getTimeBounds(&start, &end) == true)
        #expect(start == 0)
        #expect(end == 0)

        var rb = core_audio.TemporalRingBuffer()
        let std2ch = AudioStreamBasicDescription(mSampleRate: 44100, mFormatID: kAudioFormatLinearPCM, mFormatFlags: kAudioFormatFlagsNativeFloatPacked|kAudioFormatFlagIsNonInterleaved, mBytesPerPacket: 8, mFramesPerPacket: 8, mBytesPerFrame: 8, mChannelsPerFrame: 2, mBitsPerChannel: 32, mReserved: 0)
        #expect(rb.allocate(std2ch, 512) == true)
        #expect(rb.capacity() == 511)
        #expect(rb.unusedSpace() == rb.capacity())

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

        #expect(rb.allocate(mono8bit, 256) == true)

        #expect(rb.write(input_abl.unsafePointer, 128, 0) == true)
        #expect(rb.getTimeBounds(&start, &end) == true)
        #expect(start == 0)
        #expect(end == 128)

        #expect(rb.read(output_abl.unsafeMutablePointer, 128, 0) == true)
        #expect(output_abl[0].mDataByteSize == 128)
        #expect(memcmp(input_abl[0].mData, output_abl[0].mData, 128) == 0)

        #expect(rb.read(output_abl.unsafeMutablePointer, 128, 64) == true)
        #expect(output_abl[0].mDataByteSize == 64)

        #expect(memcmp(input_abl[0].mData?.advanced(by: 64), output_abl[0].mData, 64) == 0)
    }
}
