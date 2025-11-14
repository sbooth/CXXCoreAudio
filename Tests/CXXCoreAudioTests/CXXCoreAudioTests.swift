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
}
