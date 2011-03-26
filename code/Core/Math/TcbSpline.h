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
	typename Time,
	typename Control,
	typename Value
>
struct TcbSplineAccessor
{
	static inline Time time(const Key& key)
	{
		return Time(key.T);
	}

	static inline Control tension(const Key& key)
	{
		return Value(key.tension);
	}

	static inline Control continuity(const Key& key)
	{
		return Value(key.continuity);
	}

	static inline Control bias(const Key& key)
	{
		return Value(key.bias);
	}

	static inline Value value(const Key& key)
	{
		return Value(key.value);
	}

	static inline Value combine(
		const Value& v0, const Time& w0,
		const Value& v1, const Time& w1,
		const Value& v2, const Time& w2,
		const Value& v3, const Time& w3
	)
	{
		return Value(v0 * w0 + v1 * w1 + v2 * w2 + v3 * w3);
	}
};

/*! \brief Tension-Continuity-Bias spline evaluator.
 * \ingroup Core
 */
template <
	typename Key,
	typename Time,
	typename Control,
	typename Value,
	typename Accessor = TcbSplineAccessor< Key, Time, Control, Value >
>
class TcbSpline : public ISpline< Key, Time, Value >
{
public:
	TcbSpline(const Accessor& accessor)
	:	m_accessor(accessor)
	{
	}

	virtual Value evaluate(const Time& Tat, const Time& Tend = Time(-1.0f), const Time& stiffness = Time(0.5f)) const
	{
		int32_t index = m_accessor.index(Tat);

		int32_t index_n1 = m_accessor.index(index - 1);
		int32_t index_1 = m_accessor.index(index + 1);
		int32_t index_2 = m_accessor.index(index + 2);

		Time T0(m_accessor.time(index));
		Time T1(m_accessor.time(index_1));
		T_ASSERT (T0 < T1);

		Control t = m_accessor.tension(index);
		Control c = m_accessor.continuity(index);
		Control b = m_accessor.bias(index);

		const static Control c_one(1);
		const static Control c_two(2);

		Control k11 = ((c_one - t) * (c_one + b) * (c_one + c)) / c_two;
		Control k21 = ((c_one - t) * (c_one + b) * (c_one - c)) / c_two;
		Control k12 = ((c_one - t) * (c_one - b) * (c_one - c)) / c_two;
		Control k22 = ((c_one - t) * (c_one - b) * (c_one + c)) / c_two;

		Value v0 = m_accessor.value(index);
		Value v1 = m_accessor.value(index_1);
		Value vp = m_accessor.value(index_n1);
		Value vn = m_accessor.value(index_2);

		Value d0 = m_accessor.combine(
			v0, k11,
			vp, -k11,
			v1, k12,
			v0, -k12
		);

		Value d1 = m_accessor.combine(
			v1, k21,
			v0, -k21,
			vn, k22,
			v1, -k22
		);

		Time T = (T1 > T0) ? (Tat - T0) / (T1 - T0) : Time(0.0f);
		Time T2 = T * T;
		Time T3 = T2 * T;

		Time h1 =  Time(2.0f) * T3 - Time(3.0f) * T2 + Time(1.0f);
		Time h2 = -Time(2.0f) * T3 + Time(3.0f) * T2;
		Time h3 = T3 - Time(2.0f) * T2 + T;
		Time h4 = T3 - T2;

		return m_accessor.combine(
			v0, h1,
			v1, h2,
			d0, h3,
			d1, h4
		);
	}

private:
	Accessor m_accessor;
};

}

#endif	// traktor_Hermite_H
