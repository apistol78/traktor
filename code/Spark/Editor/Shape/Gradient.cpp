#include "Spark/Editor/Shape/Gradient.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Gradient", Gradient, Object)

Gradient::Gradient(GradientType gradientType)
:	m_gradientType(gradientType)
{
}

Gradient::GradientType Gradient::getGradientType() const
{
	return m_gradientType;
}

void Gradient::addStop(float offset, unsigned long color)
{
	Stop stop = { offset, color };
	m_stops.push_back(stop);
}
	
	}
}
