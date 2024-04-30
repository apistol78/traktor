/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! System memory allocators.
 * \ingroup Core
 */
class T_DLLCLASS Alloc
{
public:
	/*! Allocate chunk of memory. */
	[[nodiscard]] static void* acquire(size_t size, const char* tag);

	/*! Free chunk of memory. */
	static void free(void* ptr);

	/*! Allocate aligned chunk of memory. */
	[[nodiscard]] static void* acquireAlign(size_t size, size_t align, const char* tag);

	/*! Free aligned chunk of memory. */
	static void freeAlign(void* ptr);

	/*! Return number of allocations. */
	static size_t count();

	/*! Return amount of memory currently allocated. */
	static size_t allocated();
};

}

