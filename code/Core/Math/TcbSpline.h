#ifndef traktor_TcbSpline_H
#define traktor_TcbSpline_H

#include "Core/Config.h"
#include "Core/Math/Const.h"
#include "Core/Math/ISpline.h"
#include "Core/Math/MathConfig.h"
#include "Core/Math/SplineControl.h"

namespace traktor
{

/*! \brief Tension-Continuity-Bias spline evaluator.
 * \ingroup Core
 */
template <
	typename Key,
	typename Time,
	typename Control,
	typename Value,
	typename Accessor
>
class T_MATH_ALIGN16 TcbSpline : public ISpline< Key, Time, Value >
{
public:
	TcbSpline(const Accessor& accessor)
	:	m_accessor(accessor)
	{
	}

	virtual Value evaluate(const Time& Tat, const Time& Tend = Time(-1.0f), const Time& stiffness = Time(0.5f)) const
	{
		Time Tcurr = Tat;
		while (Tcurr >= Tend)
			Tcurr -= Tend;

		int32_t index = m_accessor.index(Tcurr);
		int32_t index_n1 = index - 1;
		int32_t index_1 = index + 1;
		int32_t index_2 = index + 2;

		Time T0(m_accessor.time(index));
		Time T1(m_accessor.time(index_1));
		if (T1 < T0)
			T1 += Tend;

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

		Time T = (T1 > T0) ? (Tcurr - T0) / (T1 - T0) : Time(0.0f);
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

#endif	// traktor_TcbSpline_H
