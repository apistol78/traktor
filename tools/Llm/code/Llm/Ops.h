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

#include <functional>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_LLM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::llm
{

class GgufTensor;

/*! How rotary position embedding pairs up the channels of a head.
 *
 * Llama and Mistral rotate adjacent channels; Qwen and the GPT-NeoX lineage
 * rotate a channel against its counterpart in the other half of the head.
 * Choosing wrongly still produces fluent looking output, just output that
 * ignores word order, so it is worth getting right.
 */
enum class RopeType
{
	Normal,
	NeoX
};

/*! Split \a count items of work across the job manager.
 *
 * \a fn is called with disjoint half open ranges. Small problems run inline
 * rather than paying for scheduling.
 */
void T_DLLCLASS parallelFor(uint32_t count, uint32_t costPerItem, const std::function< void(uint32_t, uint32_t) >& fn);

/*! out = weight * x, where \a weight holds \a d rows of \a n elements. */
void T_DLLCLASS matmul(float* outResult, const GgufTensor& weight, const float* x, uint32_t n, uint32_t d);

/*! out = x * weight / sqrt(mean(x^2) + epsilon), accumulated in double. */
void T_DLLCLASS rmsNorm(float* outResult, const float* x, const float* weight, uint32_t n, float epsilon);

/*! In place softmax over \a n values, shifted by the maximum for stability. */
void T_DLLCLASS softmax(float* x, uint32_t n);

/*! x = x / (1 + exp(-x)) * y, the SwiGLU activation. */
void T_DLLCLASS siluMultiply(float* x, const float* y, uint32_t n);

/*! x += y */
void T_DLLCLASS addTo(float* x, const float* y, uint32_t n);

/*! Rotate the first \a ropeDim channels of every head in \a x for \a position.
 *
 * \param freqScale Multiplies the position, for models trained with their
 *                  context stretched linearly.
 * \param frequencyFactors Optional per channel pair divisor of length
 *                         ropeDim / 2, as Llama 3.1 and later carry in
 *                         "rope_freqs.weight": it stretches the long
 *                         wavelength channels and leaves the short ones
 *                         alone, rather than scaling every channel alike.
 *                         Null applies no per channel correction.
 */
void T_DLLCLASS rope(float* x, uint32_t headCount, uint32_t headDim, uint32_t ropeDim, int32_t position, float freqBase, float freqScale, RopeType type, const float* frequencyFactors);

}
