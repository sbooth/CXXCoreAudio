# CXXCoreAudio

| C++ Class | Description |
| --- | --- |
| [CoreAudio::CAAudioBuffer](Sources/CXXCoreAudio/include/CAAudioBuffer.hpp) | A class containing an `AudioBufferList` with a specific format, frame capacity, and frame length. |
| [CoreAudio::CAChannelLayout](Sources/CXXCoreAudio/include/CAChannelLayout.hpp) | A class simplifying use of the variable-length `AudioChannelLayout` structure. |
| [CoreAudio::CAStreamDescription](Sources/CXXCoreAudio/include/CAStreamDescription.hpp) | A class extending the functionality of an `AudioStreamBasicDescription` structure. |
| [CoreAudio::CATimeStamp](Sources/CXXCoreAudio/include/CATimeStamp.hpp) | A class extending the functionality of an `AudioTimeStamp` structure. |

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
