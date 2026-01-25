// swift-tools-version: 5.9
//
// SPDX-FileCopyrightText: 2025 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXCoreAudio
//

import PackageDescription

let package = Package(
    name: "CXXCoreAudio",
    products: [
        .library(
            name: "CXXCoreAudio",
            targets: [
                "CXXCoreAudio",
            ]
        ),
    ],
    dependencies: [
        .package(url: "https://github.com/sbooth/CXXCFRef", .upToNextMinor(from: "0.2.0")),
    ],
    targets: [
        .target(
            name: "CXXCoreAudio",
            dependencies: [
                "CXXCFRef",
            ],
            cxxSettings: [
                .headerSearchPath("include/CXXCoreAudio"),
            ],
            linkerSettings: [
                .linkedFramework("CoreAudio"),
                .linkedFramework("AudioToolbox"),
            ],
        ),
        .testTarget(
            name: "CXXCoreAudioTests",
            dependencies: [
                "CXXCoreAudio",
            ],
            swiftSettings: [
                .interoperabilityMode(.Cxx),
            ]
        ),
    ],
    cxxLanguageStandard: .cxx17
)
