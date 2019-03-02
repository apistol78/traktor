#include "Spray/Trail.h"
#include "Spray/TrailInstance.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.Trail", Trail, Object)

Trail::Trail(
	const resource::Proxy< render::Shader >& shader,
	float width,
	float age,
	float lengthThreshold,
	float breakThreshold
)
:	m_shader(shader)
,	m_width(width)
,	m_age(age)
,	m_lengthThreshold(lengthThreshold)
,	m_breakThreshold(breakThreshold)
{
}

Ref< TrailInstance > Trail::createInstance() const
{
	return new TrailInstance(m_shader, m_width, m_age, m_lengthThreshold, m_breakThreshold);
}

	}
}
