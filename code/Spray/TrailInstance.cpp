#include "Render/Shader.h"
#include "Spray/TrailInstance.h"
#include "Spray/TrailRenderer.h"

namespace traktor
{
	namespace spray
	{
		namespace
		{

const Scalar c_lengthThreshold(0.1f);
const Scalar c_breakThreshold(10.0f);

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.TrailInstance", TrailInstance, Object)

TrailInstance::TrailInstance(
	const resource::Proxy< render::Shader >& shader,
	float width,
	float lengthThreshold,
	float breakThreshold
)
:	m_shader(shader)
,	m_width(width)
,	m_lengthThreshold(lengthThreshold)
,	m_breakThreshold(breakThreshold)
{
	m_points.push_back(Vector4::zero());
	m_last = Vector4::zero();
}

void TrailInstance::update(Context& context, const Transform& transform, bool enable)
{
	if (!enable)
		return;

	Vector4 position = transform.translation().xyz1();

	Scalar ln = (position - m_last).length2();
	if (ln >= m_lengthThreshold * m_lengthThreshold)
	{
		if (m_breakThreshold > FUZZY_EPSILON && ln >= m_breakThreshold * m_breakThreshold)
			m_points.clear();
		m_points.push_back(position);
		m_last = position;
	}
	else
		m_points.back() = position;
}

void TrailInstance::render(TrailRenderer* trailRenderer, const Transform& transform, const Vector4& cameraPosition, const Plane& cameraPlane)
{
	trailRenderer->render(
		m_shader,
		m_points,
		cameraPosition,
		cameraPlane,
		m_width,
		m_lengthThreshold
	);
}

	}
}
