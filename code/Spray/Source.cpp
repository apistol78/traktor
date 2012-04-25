#include "Spray/Source.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.Source", Source, Object)

Source::Source(
	float constantRate,
	float velocityRate
)
:	m_constantRate(constantRate)
,	m_velocityRate(velocityRate)
{
}

	}
}
