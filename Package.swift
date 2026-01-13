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
		// Products define the executables and libraries a package produces, making them visible to other packages.
		.library(
			name: "CXXCoreAudio",
			targets: [
				"CXXCoreAudio",
			]
		),
	],
	dependencies: [
		.package(url: "https://github.com/sbooth/CXXCFRef", from: "0.1.0"),
	],
	targets: [
		// Targets are the basic building blocks of a package, defining a module or a test suite.
		// Targets can depend on other targets in this package and products from dependencies.
		.target(
			name: "CXXCoreAudio",
			dependencies: [
				"CXXCFRef",
			],
			cSettings: [
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
