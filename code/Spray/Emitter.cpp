#include "Render/Shader.h"
#include "Spray/Emitter.h"
#include "Spray/EmitterInstance.h"
#include "Spray/Modifier.h"
#include "Spray/Source.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.Emitter", Emitter, Object)

Emitter::Emitter(
	Source* source,
	const RefArray< Modifier >& modifiers,
	const resource::Proxy< render::Shader >& shader,
	float middleAge,
	float cullNearDistance,
	float fadeNearRange,
	float warmUp,
	bool sort,
	bool worldSpace
)
:	m_source(source)
,	m_modifiers(modifiers)
,	m_shader(shader)
,	m_middleAge(middleAge)
,	m_cullNearDistance(cullNearDistance)
,	m_fadeNearRange(fadeNearRange)
,	m_warmUp(warmUp)
,	m_sort(sort)
,	m_worldSpace(worldSpace)
{
}

Ref< EmitterInstance > Emitter::createInstance(float duration) const
{
	return new EmitterInstance(this, duration);
}

	}
}
