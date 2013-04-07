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
	typename Value,
	typename Accessor
>
class T_MATH_ALIGN16 TcbSpline : public ISpline< Value >
{
public:
	TcbSpline(const Accessor& accessor, float Tend = -1.0f)
	:	m_accessor(accessor)
	,	m_Tend(Tend)
	{
	}

	virtual Value evaluate(float T) const
	{
		int32_t index = m_accessor.index(T, m_Tend);
		int32_t index_n1 = index - 1;
		int32_t index_1 = index + 1;
		int32_t index_2 = index + 2;

		float T0 = m_accessor.time(index);
		float T1 = m_accessor.time(index_1);

		if (T0 > T1)
		{
			if (T > T0)
			{
				T1 += m_Tend;
			}
			else
			{
				T0 -= m_Tend;
			}
		}

		float t = m_accessor.tension(index);
		float c = m_accessor.continuity(index);
		float b = m_accessor.bias(index);

		float one_t = 1.0f - t;
		float bc = b * c;

		float k11 = (one_t * (1.0f + c + b + bc)) / 2.0f;
		float k21 = (one_t * (1.0f - c + b - bc)) / 2.0f;
		float k12 = (one_t * (1.0f - c - b + bc)) / 2.0f;
		float k22 = (one_t * (1.0f + c - b - bc)) / 2.0f;

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

		float Tc = (T1 > T0) ? (T - T0) / (T1 - T0) : 0.0f;
		float T2 = Tc * Tc;
		float T3 = T2 * Tc;

		float h1 =  2.0f * T3 - 3.0f * T2 + 1.0f;
		float h2 = -2.0f * T3 + 3.0f * T2;
		float h3 = T3 - 2.0f * T2 + Tc;
		float h4 = T3 - T2;

		return m_accessor.combine(
			v0, h1,
			v1, h2,
			d0, h3,
			d1, h4
		);
	}

private:
	Accessor m_accessor;
	float m_Tend;
};

}

#endif	// traktor_TcbSpline_H
