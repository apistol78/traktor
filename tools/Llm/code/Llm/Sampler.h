/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Random.h"
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_LLM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::llm
{

/*! How the next token is drawn from the model's output distribution. */
class T_DLLCLASS SamplerSettings
{
public:
	float temperature = 0.8f;  //!< Zero or less always takes the most likely token.
	int32_t topK = 40;		   //!< Keep at most this many candidates; zero disables.
	float topP = 0.95f;		   //!< Keep the smallest set with at least this much mass.
	float repeatPenalty = 1.1f; //!< One disables; higher discourages recent tokens.
	int32_t repeatWindow = 64; //!< How far back the penalty looks.
	uint32_t seed = 0;		   //!< Zero picks an arbitrary seed on create.
};

/*! Draws the next token from a row of logits.
 * \ingroup Llm
 */
class T_DLLCLASS Sampler : public Object
{
	T_RTTI_CLASS;

public:
	explicit Sampler(const SamplerSettings& settings = SamplerSettings());

	void setSettings(const SamplerSettings& settings);

	const SamplerSettings& getSettings() const { return m_settings; }

	/*! Pick a token from \a logits, which is modified in place.
	 *
	 * \param history Tokens generated so far, used by the repetition penalty.
	 */
	int32_t sample(float* logits, int32_t count, const AlignedVector< int32_t >& history);

private:
	class Candidate
	{
	public:
		int32_t token;
		float probability;
	};

	SamplerSettings m_settings;
	Random m_random;
	bool m_seeded = false;
	AlignedVector< Candidate > m_candidates;
};

}
