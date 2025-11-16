# CXXCoreAudio

Assorted C++ classes simplifying common operations in Core Audio.

| Class | Description |
| --- | --- |
| [CAAudioBuffer](Sources/CXXCoreAudio/include/CAAudioBuffer.hpp) | An `AudioBufferList` with a specific format, frame capacity, and frame length. |
| [CAAudioRingBuffer](Sources/CXXCoreAudio/include/CAAudioRingBuffer.hpp) | A lock-free SPSC audio ring buffer supporting non-interleaved audio. |
| [CAChannelLayout](Sources/CXXCoreAudio/include/CAChannelLayout.hpp) | Simplifies use of the variable-length `AudioChannelLayout`. |
| [CAStreamDescription](Sources/CXXCoreAudio/include/CAStreamDescription.hpp) | Extends the functionality of an `AudioStreamBasicDescription`. |
| [CATimeStamp](Sources/CXXCoreAudio/include/CATimeStamp.hpp) | Extends the functionality of an `AudioTimeStamp`. |
| [CAValueRange](Sources/CXXCoreAudio/include/CAValueRange.hpp) | Extends the functionality of an `AudioValueRange`. |

> [!NOTE]
> C++17 is required.

All classes are in the `CXXCoreAudio` namespace.

## Installation

### Swift Package Manager

Add a package dependency to https://github.com/sbooth/CXXCoreAudio in Xcode.

### Manual or Custom Build

1. Clone the [CXXCoreAudio](https://github.com/sbooth/CXXCoreAudio) repository.
2. `swift build`.

## License

Released under the [MIT License](https://github.com/sbooth/CXXCoreAudio/blob/main/LICENSE.txt).
