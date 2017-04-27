/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spark_SvgGradient_H
#define traktor_spark_SvgGradient_H

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Color4ub.h"

namespace traktor
{
	namespace spark
	{

/*! \brief
 * \ingroup Spark
 */
class SvgGradient : public Object
{
	T_RTTI_CLASS;
	
public:
	enum GradientType
	{
		GtLinear,
		GtRadial
	};

	struct Stop
	{
		float offset;
		Color4ub color;
	};

	SvgGradient(GradientType gradientType);
	
	GradientType getGradientType() const;
	
	void addStop(float offset, const Color4ub& color);

	const AlignedVector< Stop >& getStops() const;
	
private:
	GradientType m_gradientType;
	AlignedVector< Stop > m_stops;
};
	
	}
}

#endif	// traktor_spark_SvgGradient_H
