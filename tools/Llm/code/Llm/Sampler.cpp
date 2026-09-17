/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Llm/Sampler.h"

#include "Core/Date/DateTime.h"
#include "Llm/Ops.h"

#include <algorithm>

namespace traktor::llm
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.llm.Sampler", Sampler, Object)

Sampler::Sampler(const SamplerSettings& settings)
{
	setSettings(settings);
}

void Sampler::setSettings(const SamplerSettings& settings)
{
	const bool reseed = !m_seeded || m_settings.seed != settings.seed;

	m_settings = settings;

	if (reseed)
	{
		uint32_t seed = settings.seed;
		if (seed == 0)
			seed = (uint32_t)DateTime::now().getSecondsSinceEpoch() ^ 0x9e3779b9;

		m_random = Random(seed);
		m_seeded = true;
	}
}

int32_t Sampler::sample(float* logits, int32_t count, const AlignedVector< int32_t >& history)
{
	if (count <= 0)
		return -1;

	// Discourage whatever was said recently. Positive logits are divided and
	// negative ones multiplied, so the penalty always moves a token down.
	if (m_settings.repeatPenalty > 1.0f && m_settings.repeatWindow > 0)
	{
		const int32_t from = std::max(0, (int32_t)history.size() - m_settings.repeatWindow);
		for (int32_t i = from; i < (int32_t)history.size(); ++i)
		{
			const int32_t token = history[i];
			if (token < 0 || token >= count)
				continue;

			if (logits[token] > 0.0f)
				logits[token] /= m_settings.repeatPenalty;
			else
				logits[token] *= m_settings.repeatPenalty;
		}
	}

	if (m_settings.temperature <= 0.0f)
	{
		int32_t best = 0;
		for (int32_t i = 1; i < count; ++i)
		{
			if (logits[i] > logits[best])
				best = i;
		}
		return best;
	}

	const float scale = 1.0f / m_settings.temperature;
	for (int32_t i = 0; i < count; ++i)
		logits[i] *= scale;

	softmax(logits, (uint32_t)count);

	m_candidates.resize(count);
	for (int32_t i = 0; i < count; ++i)
		m_candidates[i] = { i, logits[i] };

	int32_t keep = count;

	// Top k first; it bounds the sort that nucleus sampling needs.
	if (m_settings.topK > 0 && m_settings.topK < keep)
	{
		std::partial_sort(m_candidates.begin(), m_candidates.begin() + m_settings.topK, m_candidates.begin() + keep, [](const Candidate& a, const Candidate& b) {
			return a.probability > b.probability;
		});
		keep = m_settings.topK;
	}
	else
	{
		std::sort(m_candidates.begin(), m_candidates.begin() + keep, [](const Candidate& a, const Candidate& b) {
			return a.probability > b.probability;
		});
	}

	// Nucleus: keep the shortest prefix holding at least topP of the mass.
	if (m_settings.topP < 1.0f)
	{
		float cumulative = 0.0f;
		for (int32_t i = 0; i < keep; ++i)
		{
			cumulative += m_candidates[i].probability;
			if (cumulative >= m_settings.topP)
			{
				keep = i + 1;
				break;
			}
		}
	}

	float total = 0.0f;
	for (int32_t i = 0; i < keep; ++i)
		total += m_candidates[i].probability;

	if (total <= 0.0f)
		return m_candidates[0].token;

	float target = m_random.nextFloat() * total;
	for (int32_t i = 0; i < keep; ++i)
	{
		target -= m_candidates[i].probability;
		if (target <= 0.0f)
			return m_candidates[i].token;
	}

	return m_candidates[keep - 1].token;
}

}
