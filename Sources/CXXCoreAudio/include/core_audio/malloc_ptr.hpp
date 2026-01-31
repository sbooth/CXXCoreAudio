//
// SPDX-FileCopyrightText: 2026 Stephen F. Booth <contact@sbooth.dev>
// SPDX-License-Identifier: MIT
//
// Part of https://github.com/sbooth/CXXCoreAudio
//

#pragma once

#include <cstdlib>
#include <memory>

namespace core_audio {

/// A std::unique_ptr deleter using std::free.
struct free_deleter {
    void operator()(void *_Nullable ptr) noexcept { std::free(ptr); }
};

/// A std::unique_ptr managing an allocation from std::malloc.
template <typename T, typename = std::enable_if_t<std::is_trivially_copyable_v<T>>>
using malloc_ptr = std::unique_ptr<T, free_deleter>;

} /* namespace core_audio */
