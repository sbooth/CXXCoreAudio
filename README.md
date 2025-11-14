# CXXCoreAudio

| C++ Class | Description |
| --- | --- |
| [CoreAudio::CAAudioBuffer](Sources/CXXCoreAudio/include/CAAudioBuffer.hpp) | An `AudioBufferList` with a specific format, frame capacity, and frame length. |
| [CoreAudio::CAChannelLayout](Sources/CXXCoreAudio/include/CAChannelLayout.hpp) | Simplifies use of the variable-length `AudioChannelLayout`. |
| [CoreAudio::CAStreamDescription](Sources/CXXCoreAudio/include/CAStreamDescription.hpp) | Extends the functionality of an `AudioStreamBasicDescription`. |
| [CoreAudio::CATimeStamp](Sources/CXXCoreAudio/include/CATimeStamp.hpp) | Extends the functionality of an `AudioTimeStamp`. |

> [!NOTE]
> C++17 is required.

## Installation

### Swift Package Manager

Add a package dependency to https://github.com/sbooth/CXXCoreAudio in Xcode.

### Manual or Custom Build

1. Clone the [CXXCoreAudio](https://github.com/sbooth/CXXCoreAudio) repository.
2. `swift build`.

## License

Released under the [MIT License](https://github.com/sbooth/CXXCoreAudio/blob/main/LICENSE.txt).
