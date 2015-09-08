#ifndef traktor_spark_Gradient_H
#define traktor_spark_Gradient_H

#include "Core/Object.h"

namespace traktor
{
	namespace spark
	{

class Gradient : public Object
{
	T_RTTI_CLASS;
	
public:
	enum GradientType
	{
		GtLinear,
		GtRadial
	};
	
	Gradient(GradientType gradientType);
	
	GradientType getGradientType() const;
	
	void addStop(float offset, unsigned long color);
	
private:
	struct Stop
	{
		float offset;
		unsigned long color;
	};
	
	GradientType m_gradientType;
	std::vector< Stop > m_stops;
};
	
	}
}

#endif	// traktor_spark_Gradient_H
