/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Llm/GgmlType.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_LLM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::llm
{

/*! Decode \a count elements of \a type from \a src into \a dst.
 * \ingroup Llm
 *
 * \a count must be an integer multiple of the block size of \a type,
 * and \a dst must have room for \a count floats. Unsupported types
 * write nothing.
 */
void T_DLLCLASS dequantize(GgmlType type, const void* src, float* dst, uint32_t count);

/*! Dot product of a stored row against a plain float vector.
 * \ingroup Llm
 *
 * \a row points at the first block of a row of \a count elements of
 * \a type; \a v holds \a count floats. Blocks are decoded on the fly so
 * no dequantized copy of the row is ever materialized.
 */
float T_DLLCLASS dotRow(GgmlType type, const void* row, const float* v, uint32_t count);

/*! Dot product of two float vectors, four lanes at a time. */
float T_DLLCLASS dotF32(const float* a, const float* b, uint32_t count);

}
