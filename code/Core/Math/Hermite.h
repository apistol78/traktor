#ifndef traktor_Hermite_H
#define traktor_Hermite_H

#include "Core/Config.h"
#include "Core/Math/Const.h"
#include "Core/Math/ISpline.h"
#include "Core/Math/SplineControl.h"

namespace traktor
{

/*! \brief Default element accessor.
 * \ingroup Core
 */
template <
	typename Key,
	typename Value
>
struct HermiteAccessor
{
	static inline float time(const Key* keys, size_t nkeys, const Key& key)
	{
		return key.T;
	}

	static inline Value value(const Key& key)
	{
		return Value(key.value);
	}

	static inline Value combine(
		const Value& v0, float w0,
		const Value& v1, float w1,
		const Value& v2, float w2,
		const Value& v3, float w3
	)
	{
		return Value(v0 * w0 + v1 * w1 + v2 * w2 + v3 * w3);
	}
};

/*! \brief Hermite spline evaluator.
 * \ingroup Core
 */
template <
	typename Key,
	typename Value,
	typename Accessor = HermiteAccessor< Key, Value >,
	typename TimeControl = ClampTime
>
class Hermite : public ISpline< Value >
{
public:
	Hermite(const Key* keys, size_t nkeys, float Tend = -1.0f, float stiffness = 0.5f)
	:	m_keys(keys)
	,	m_nkeys(nkeys)
	,	m_Tend(Tend)
	,	m_stiffness(stiffness)
	{
	}

	virtual Value evaluate(float T) const
	{
		T_ASSERT (m_nkeys >= 2);

		float Tfirst = Accessor::time(m_keys, m_nkeys, m_keys[0]);
		float Tlast = Accessor::time(m_keys, m_nkeys, m_keys[m_nkeys - 1]);
		float Tcurr = TimeControl::t(T, Tfirst, Tlast, m_Tend > 0.0f ? m_Tend : Tlast);

		int index = 0;
		while (index < int(m_nkeys - 1) && Tcurr >= Accessor::time(m_keys, m_nkeys, m_keys[index + 1]))
			++index;

		int index_n1 = TimeControl::index(index - 1, int(m_nkeys - 1));
		int index_1 = TimeControl::index(index + 1, int(m_nkeys - 1));
		int index_2 = TimeControl::index(index + 2, int(m_nkeys - 1));
		
		const Key& cp0 = m_keys[index];
		const Key& cp1 = m_keys[index_1];

		Value v0 = Accessor::value(cp0);
		Value v1 = Accessor::value(cp1);

		float t0 = Accessor::time(m_keys, m_nkeys, cp0);
		float t1 = Accessor::time(m_keys, m_nkeys, cp1);

		if (t0 >= t1)
			t1 = m_Tend;

		const Key& cpp = m_keys[index_n1];
		const Key& cpn = m_keys[index_2];

		Value vp = Accessor::value(cpp);
		Value vn = Accessor::value(cpn);

		float t = (Tcurr - t0) / (t1 - t0);
		float t2 = t * t;
		float t3 = t2 * t;

		float h2 = 3.0f * t2 - t3 - t3;
		float h1 = 1.0f - h2;
		float h4 = t3 - t2;
		float h3 = h4 - t2 + t;

		h3 *= m_stiffness;
		h4 *= m_stiffness;

		return Accessor::combine(
			v0, h1 - h4,
			v1, h2 + h3,
			vp, -h3,
			vn, h4
		);
	}

private:
	const Key* m_keys;
	size_t m_nkeys;
	float m_Tend;
	float m_stiffness;
};

}

#endif	// traktor_Hermite_H
