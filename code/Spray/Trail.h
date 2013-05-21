#ifndef traktor_spray_Trail_H
#define traktor_spray_Trail_H

#include "Core/Object.h"
#include "Resource/Proxy.h"

namespace traktor
{
	namespace render
	{

class Shader;

	}

	namespace spray
	{

class TrailInstance;

class Trail : public Object
{
	T_RTTI_CLASS;

public:
	Trail(
		const resource::Proxy< render::Shader >& shader,
		float width,
		float lengthThreshold,
		float breakThreshold
	);

	Ref< TrailInstance > createInstance() const;

private:
	resource::Proxy< render::Shader > m_shader;
	float m_width;
	float m_lengthThreshold;
	float m_breakThreshold;
};

	}
}

#endif	// traktor_spray_Trail_H
