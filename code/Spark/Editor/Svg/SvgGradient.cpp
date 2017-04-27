/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Spark/Editor/Svg/SvgGradient.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.SvgGradient", SvgGradient, Object)

SvgGradient::SvgGradient(GradientType gradientType)
:	m_gradientType(gradientType)
{
}

SvgGradient::GradientType SvgGradient::getGradientType() const
{
	return m_gradientType;
}

void SvgGradient::addStop(float offset, const Color4ub& color)
{
	Stop stop = { offset, color };
	m_stops.push_back(stop);
}

const AlignedVector< SvgGradient::Stop >& SvgGradient::getStops() const
{
	return m_stops;
}

	}
}
