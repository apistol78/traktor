#ifndef traktor_Hermite_H
#define traktor_Hermite_H

#include "Core/Config.h"
#include "Core/Math/Const.h"
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
	typename Accessor = TcbSplineAccessor< Key, Time, Control, Value >,
	typename TimeControl = ClampTime< Time >
>
struct TcbSpline
{
	static Value evaluate(const Key* keys, size_t nkeys, const Time& Tat, const Time& Tend = Time(-1.0f), const Time& stiffness = Time(0.5f))
	{
		T_ASSERT (nkeys >= 2);

		Time Tfirst(Accessor::time(keys[0]));
		Time Tlast(Accessor::time(keys[nkeys - 1]));
		Time Tcurr = TimeControl::t(Tat, Tfirst, Tlast, Tend > Time(0.0f) ? Tend : Tlast);

		int index = 0;
		while (index < int(nkeys - 1) && Tcurr >= Accessor::time(keys[index + 1]))
			++index;

		int index_n1 = TimeControl::index(index - 1, int(nkeys));
		int index_1 = TimeControl::index(index + 1, int(nkeys));
		int index_2 = TimeControl::index(index + 2, int(nkeys));
		
		const Key& cp0 = keys[index];
		const Key& cp1 = keys[index_1];
		const Key& cpp = keys[index_n1];
		const Key& cpn = keys[index_2];

		Time T0(Accessor::time(cp0));
		Time T1(Accessor::time(cp1));

		if (T0 >= T1)
			T1 = Tend;

		Control t = Accessor::tension(cp0);
		Control c = Accessor::continuity(cp0);
		Control b = Accessor::bias(cp0);

		const static Control c_one(1);
		const static Control c_two(2);

		Control k11 = ((c_one - t) * (c_one + b) * (c_one + c)) / c_two;
		Control k21 = ((c_one - t) * (c_one + b) * (c_one - c)) / c_two;
		Control k12 = ((c_one - t) * (c_one - b) * (c_one - c)) / c_two;
		Control k22 = ((c_one - t) * (c_one - b) * (c_one + c)) / c_two;

		Value v0 = Accessor::value(cp0);
		Value v1 = Accessor::value(cp1);
		Value vp = Accessor::value(cpp);
		Value vn = Accessor::value(cpn);

		Value d0 = Accessor::combine(
			v0, k11,
			vp, -k11,
			v1, k12,
			v0, -k12
		);

		Value d1 = Accessor::combine(
			v1, k21,
			v0, -k21,
			vn, k22,
			v1, -k22
		);

		Time T = (Tcurr - T0) / (T1 - T0);
		Time T2 = T * T;
		Time T3 = T2 * T;

		Time h1 =  Time(2.0f) * T3 - Time(3.0f) * T2 + Time(1.0f);
		Time h2 = -Time(2.0f) * T3 + Time(3.0f) * T2;
		Time h3 = T3 - Time(2.0f) * T2 + T;
		Time h4 = T3 - T2;

		return Accessor::combine(
			v0, h1,
			v1, h2,
			d0, h3,
			d1, h4
		);
	}
};

}

#endif	// traktor_Hermite_H
