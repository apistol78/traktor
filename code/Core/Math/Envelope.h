/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <utility>
#include "Core/Containers/AlignedVector.h"

namespace traktor
{

/*! Linear evaluator.
 * \ingroup Core
 */
template < typename KeyType >
struct LinearEvaluator
{
	typedef std::pair< float, KeyType > key_value_t;

	static KeyType evaluate(const AlignedVector< key_value_t >& keys, float T, int hint)
	{
		const key_value_t& cp0 = keys[hint];
		const key_value_t& cp1 = keys[hint + 1];
		const float b = (T - cp0.first) / (cp1.first - cp0.first);
		return cp0.second * (1.0f - b) + cp1.second * b;
	}
};

/*! Hermite spline evaluator.
 * \ingroup Core
 */
template < typename KeyType >
struct HermiteEvaluator
{
	typedef std::pair< float, KeyType > key_value_t;

	static KeyType evaluate(const AlignedVector< key_value_t >& keys, float T, int hint)
	{
		const key_value_t& cp0 = keys[hint];
		const key_value_t& cp1 = keys[hint + 1];

		if (T == cp0.first)
			return cp0.second;
		if (T == cp1.first)
			return cp1.second;

		key_value_t cpp, cpn;
		if (hint > 0)
			cpp = keys[hint - 1];
		else
		{
			cpp.first = -1.0f;
			cpp.second = cp0.second - (cp1.second - cp0.second);
		}
		if (hint < (int)(keys.size() - 2))
			cpn = keys[hint + 2];
		else
		{
			cpn.first = 2.0f;
			cpn.second = cp1.second + (cp1.second - cp0.second);
		}

		const float t = (T - cp0.first) / (cp1.first - cp0.first);
		const float t2 = t * t;
		const float t3 = t2 * t;

		const float h1 = 2.0f * t3 - 3.0f * t2 + 1.0f;
		const float h2 = -2.0f * t3 + 3.0f * t2;
		const float h3 = t3 - 2.0f * t2 + t;
		const float h4 = t3 - t2;

		// Catmull-Rom splines.
		const float c_stiffness = 0.5f;
		const KeyType T0 = c_stiffness * (cp1.second - cpp.second);
		const KeyType T1 = c_stiffness * (cpn.second - cp0.second);

		return h1 * cp0.second + h2 * cp1.second + h3 * T0 + h4 * T1;
	}
};

/*! Spline envelope.
 * \ingroup Core
 */
template < typename KeyType, typename Evaluator = HermiteEvaluator< KeyType > >
class Envelope
{
public:
	typedef std::pair< float, KeyType > key_value_t;

	Envelope() = default;

	explicit Envelope(const AlignedVector< key_value_t >& keys)
	:	m_keys(keys)
	{
	}

	void removeAllKeys()
	{
		m_keys.resize(0);
	}

	void addKey(float T, const KeyType& key)
	{
		m_keys.push_back(key_value_t(T, key));
	}

	void setKeys(const AlignedVector< key_value_t >& keys)
	{
		m_keys = keys;
	}

	const AlignedVector< key_value_t >& getKeys() const
	{
		return m_keys;
	}

	KeyType operator () (float T) const
	{
		if (T <= m_keys.front().first)
			return m_keys.front().second;

		if (T >= m_keys.back().first)
			return m_keys.back().second;

		int32_t hint = m_hint;
		if (hint >= 0)
		{
			if (T < m_keys[hint].first || T > m_keys[hint + 1].first)
				hint = -1;
		}
		if (hint < 0)
		{
			while (T > m_keys[hint + 1].first)
				++hint;
			m_hint = hint;
		}

		return Evaluator::evaluate(m_keys, T, hint);
	}

private:
	AlignedVector< key_value_t > m_keys;
	mutable int32_t m_hint = -1;
};

}

