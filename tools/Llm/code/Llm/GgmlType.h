/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Config.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_LLM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::llm
{

/*! Element type of a GGUF tensor.
 * \ingroup Llm
 *
 * Values are defined by the GGUF container and must not be renumbered;
 * they are read verbatim from the tensor directory of the file.
 */
enum class GgmlType : uint32_t
{
	F32 = 0,
	F16 = 1,
	Q4_0 = 2,
	Q4_1 = 3,
	Q5_0 = 6,
	Q5_1 = 7,
	Q8_0 = 8,
	Q8_1 = 9,
	Q2_K = 10,
	Q3_K = 11,
	Q4_K = 12,
	Q5_K = 13,
	Q6_K = 14,
	Q8_K = 15,
	Invalid = 0xffffffff
};

/*! Number of elements in a single block of \a type. */
uint32_t T_DLLCLASS getBlockSize(GgmlType type);

/*! Number of bytes occupied by a single block of \a type. */
uint32_t T_DLLCLASS getBlockBytes(GgmlType type);

/*! True if this build is able to decode \a type. */
bool T_DLLCLASS isSupported(GgmlType type);

/*! Human readable name of \a type, for diagnostics. */
std::wstring T_DLLCLASS getTypeName(GgmlType type);

/*! Number of bytes needed to store \a elementCount elements of \a type. */
uint64_t T_DLLCLASS getStorageSize(GgmlType type, uint64_t elementCount);

}
