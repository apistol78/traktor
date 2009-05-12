#ifndef traktor_Envelope_H
#define traktor_Envelope_H

#include <vector>
#include <utility>

namespace traktor
{

/*! \brief Linear evaluator.
 * \ingroup Core
 */
template < typename KeyType >
struct LinearEvaluator
{
	static KeyType evaluate(const std::vector< std::pair< float, KeyType > >& keys, float T, int hint)
	{
		const std::pair< float, KeyType >& cp0 = keys[hint];
		const std::pair< float, KeyType >& cp1 = keys[hint + 1];
		float b = (T - cp0.first) / (cp1.first - cp0.first);
		return cp0 * (1.0f - b) + cp1 * b;
	}
};

/*! \brief Hermite spline evaluator.
 * \ingroup Core
 */
template < typename KeyType, int Stiffness = 50 >
struct HermiteEvaluator
{
	static KeyType evaluate(const std::vector< std::pair< float, KeyType > >& keys, float T, int hint)
	{
		const std::pair< float, KeyType >& cp0 = keys[hint];
		const std::pair< float, KeyType >& cp1 = keys[hint + 1];

		if (T == cp0.first)
			return cp0.second;
		if (T == cp1.first)
			return cp1.second;

		const std::pair< float, KeyType >& cpp = keys[(hint > 0) ? hint - 1 : 0];
		const std::pair< float, KeyType >& cpn = keys[(hint < int(keys.size() - 2)) ? hint + 2 : keys.size() - 1];

		float t = (T - cp0.first) / (cp1.first - cp0.first);
		float t2 = t * t;
		float t3 = t2 * t;

		float h2 = 3.0f * t2 - t3 - t3;
		float h1 = 1.0f - h2;
		float h4 = t3 - t2;
		float h3 = h4 - t2 + t;

		KeyType T0 = (Stiffness / 100.0f) * (cp1.second - cpp.second);
		KeyType T1 = (Stiffness / 100.0f) * (cpn.second - cp0.second);

		return h1 * cp0.second + h2 * cp1.second + h3 * T0 + h4 * T1;
	}
};

/*! \brief Spline envelope.
 * \ingroup Core
 */
template < typename KeyType, typename Evaluator = HermiteEvaluator< KeyType > >
class Envelope
{
public:
	typedef std::pair< float, KeyType > KeyPair;

	Envelope() :
		m_hint(-1)
	{
	}

	void removeAllKeys()
	{
		m_keys.resize(0);
	}

	void addKey(float T, const KeyType& key)
	{
		m_keys.push_back(KeyPair(T, key));
	}

	void setKeys(const std::vector< KeyPair >& keys)
	{
		m_keys = keys;
	}

	const std::vector< KeyPair >& getKeys() const
	{
		return m_keys;
	}

	KeyType operator [] (float T) const
	{
		if (T <= m_keys.front().first)
			return m_keys.front().second;

		if (T >= m_keys.back().first)
			return m_keys.back().second;

		int hint = m_hint;
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
	std::vector< KeyPair > m_keys;
	mutable int m_hint;
};

}

#endif	// traktor_Envelope_H
