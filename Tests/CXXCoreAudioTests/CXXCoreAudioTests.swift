import Testing
@testable import CXXCoreAudio

@Suite struct CXXCoreAudioTests {
	@Test func timeStamp() async {
		let ts = CXXCoreAudio.CATimeStamp(22050.0)
		#expect(ts.IsValid())
		#expect(ts.SampleTimeIsValid())
		#expect(!ts.HostTimeIsValid())
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
		#expect(empty.IsEmpty())
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
		let empty = CXXCoreAudio.CAAudioRingBuffer()
		#expect(empty.AvailableReadCount() == 0)
		#expect(empty.AvailableWriteCount() == 0)
		#expect(empty.Capacity() == 0)

		var rb = CXXCoreAudio.CAAudioRingBuffer()
		let std2ch = CXXCoreAudio.CAStreamDescription(.float32, 44100, 2, false)
		#expect(rb.Allocate(std2ch, 512) == true)
		#expect(rb.AvailableReadCount() == 0)
		#expect(rb.AvailableWriteCount() == rb.Capacity())
	}
}
