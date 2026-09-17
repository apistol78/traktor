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

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_LLM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Path;

}

namespace traktor::llm
{

/*! Load \a fileName and check it end to end, writing a report to the log.
 * \ingroup Llm
 *
 * Reports the shape and storage type of every tensor, the first few decoded
 * weights of each (so a quantization layout can be compared against an
 * independent reading of the same bytes), a tokenizer round trip, and a
 * short greedy generation with its rates.
 *
 * \param tokenCount Tokens to generate; zero stops after the prompt.
 * \param contextLength Positions to allocate; zero uses the trained length.
 * \return Zero on success, non zero on the first failure.
 */
int32_t T_DLLCLASS runSelfTest(const Path& fileName, int32_t tokenCount, int32_t contextLength);

}
