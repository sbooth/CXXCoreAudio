import Testing
@testable import CXXCoreAudio

@Suite struct CXXCoreAudioTests {
	@Test func timeStamp() async {
		let ts = CoreAudio.CATimeStamp(22050.0)
		#expect(ts.IsValid())
		#expect(ts.SampleTimeIsValid())
		#expect(!ts.HostTimeIsValid())
	}

	@Test func streamDescription() async {
		let fmt = CoreAudio.CAStreamDescription(.float32, 44100, 2, false)
		#expect(fmt.IsPCM() == true)
		#expect(fmt.IsFloat() == true)
		#expect(fmt.IsInteger() == false)
		#expect(fmt.mSampleRate == 44100)
		#expect(fmt.ChannelCount() == 2)
		#expect(fmt.IsInterleaved() == false)
		#expect(fmt.IsNonInterleaved() == true)
	}

	@Test func channelLayout() async {
		let empty = CoreAudio.CAChannelLayout()
		#expect(empty.IsEmpty())
		#expect(empty.Size() == 0)
		#expect(empty.ChannelCount() == 0)
		let stereo = CoreAudio.CAChannelLayout.Stereo
		#expect(stereo.ChannelCount() == 2)
	}

	@Test func audioBuffer() async {
		let empty = CoreAudio.CAAudioBuffer()
		#expect(empty.FrameLength() == 0)
		#expect(empty.FrameCapacity() == 0)
	}
}
