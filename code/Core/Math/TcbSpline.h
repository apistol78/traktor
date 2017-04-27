/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
	TcbSpline(const Accessor& accessor)
	:	m_accessor(accessor)
	{
	}

	virtual Value evaluate(float T) const
	{
		float t, c, b;
		Value v0, v1, vp, vn;

		m_accessor.get(
			T,
			t, 
			c,
			b,
			v0,
			v1,
			vp,
			vn
		);

		float one_t = 1.0f - t;
		float bc = b * c;

		float k11 = (one_t * (1.0f + c + b + bc)) / 2.0f;
		float k21 = (one_t * (1.0f - c + b - bc)) / 2.0f;
		float k12 = (one_t * (1.0f - c - b + bc)) / 2.0f;
		float k22 = (one_t * (1.0f + c - b - bc)) / 2.0f;

		Value d0 = m_accessor.combine(
			T,
			v0, k11,
			vp, -k11,
			v1, k12,
			v0, -k12
		);

		Value d1 = m_accessor.combine(
			T,
			v1, k21,
			v0, -k21,
			vn, k22,
			v1, -k22
		);

		float T2 = T * T;
		float T3 = T2 * T;

		float h1 =  2.0f * T3 - 3.0f * T2 + 1.0f;
		float h2 = -2.0f * T3 + 3.0f * T2;
		float h3 = T3 - 2.0f * T2 + T;
		float h4 = T3 - T2;

		return m_accessor.combine(
			T,
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
