# CXXCoreAudio

Assorted C++ classes simplifying common operations in Core Audio.

| Class | Description |
| --- | --- |
| [AudioRingBuffer](Sources/CXXCoreAudio/include/core_audio/AudioRingBuffer.hpp) | A lock-free SPSC audio ring buffer supporting non-interleaved audio. |
| [BufferList](Sources/CXXCoreAudio/include/core_audio/BufferList.hpp) | An [`AudioBufferList`](https://developer.apple.com/documentation/coreaudiotypes/audiobufferlist?language=objc) with a specific format, frame capacity, and frame length. |
| [CARingBuffer](Sources/CXXCoreAudio/include/core_audio/CARingBuffer.hpp) | A lock-free timestamped SPSC audio ring buffer supporting non-interleaved audio. |
| [ChannelLayout](Sources/CXXCoreAudio/include/core_audio/ChannelLayout.hpp) | Simplifies use of the variable-length [`AudioChannelLayout`](https://developer.apple.com/documentation/coreaudiotypes/audiochannellayout?language=objc). |
| [StreamDescription](Sources/CXXCoreAudio/include/core_audio/StreamDescription.hpp) | Extends the functionality of an [`AudioStreamBasicDescription`](https://developer.apple.com/documentation/coreaudiotypes/audiostreambasicdescription?language=objc). |
| [TimeStamp](Sources/CXXCoreAudio/include/core_audio/TimeStamp.hpp) | Extends the functionality of an [`AudioTimeStamp`](https://developer.apple.com/documentation/coreaudiotypes/audiotimestamp?language=objc). |
| [ValueRange](Sources/CXXCoreAudio/include/core_audio/ValueRange.hpp) | Extends the functionality of an [`AudioValueRange`](https://developer.apple.com/documentation/coreaudiotypes/audiovaluerange?language=objc). |

> [!NOTE]
> C++17 is required.

All classes are in the `core_audio` namespace.

## Installation

### Swift Package Manager

Add a package dependency to https://github.com/sbooth/CXXCoreAudio in Xcode.

### Manual or Custom Build

1. Clone the [CXXCoreAudio](https://github.com/sbooth/CXXCoreAudio) repository.
2. `swift build`.

## License

Released under the [MIT License](https://github.com/sbooth/CXXCoreAudio/blob/main/LICENSE.txt).
