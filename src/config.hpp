/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Nihilai Collective Corp
 * https://github.com/nihilai-collective/oacc
 * src/config.hpp
 */
#pragma once

#if defined(_MSC_VER)
	#define OACC_INLINE [[msvc::forceinline]]
#elif defined(__GNUC__) || defined(__clang__)
	#define OACC_INLINE inline __attribute__((always_inline))
#else
	#define OACC_INLINE inline
#endif