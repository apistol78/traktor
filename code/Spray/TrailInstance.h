#ifndef traktor_spray_TrailInstance_H
#define traktor_spray_TrailInstance_H

#include "Core/Object.h"
#include "Core/Containers/CircularVector.h"
#include "Core/Math/Plane.h"
#include "Core/Math/Transform.h"
#include "Resource/Proxy.h"
#include "Spray/Types.h"

namespace traktor
{
	namespace render
	{

class Shader;

	}

	namespace spray
	{

class TrailRenderer;

class TrailInstance : public Object
{
	T_RTTI_CLASS;

public:
	TrailInstance(
		const resource::Proxy< render::Shader >& shader,
		float width,
		float lengthThreshold,
		float breakThreshold
	);

	void update(Context& context, const Transform& transform, bool enable);

	void render(TrailRenderer* trailRenderer, const Transform& transform, const Vector4& cameraPosition, const Plane& cameraPlane);

private:
	resource::Proxy< render::Shader > m_shader;
	float m_width;
	float m_lengthThreshold;
	float m_breakThreshold;
	CircularVector< Vector4, 16 > T_MATH_ALIGN16 m_points;
	Vector4 m_last;
	Vector4 m_time;
	Vector4 m_count;
};

	}
}

#endif	// traktor_spray_TrailInstance_H
