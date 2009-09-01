#include "Spray/EffectLayerInstance.h"
#include "Spray/EffectLayer.h"
#include "Spray/EmitterInstance.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectLayerInstance", EffectLayerInstance, Object)

EffectLayerInstance::EffectLayerInstance(const EffectLayer* layer, EmitterInstance* emitterInstance)
:	m_layer(layer)
,	m_emitterInstance(emitterInstance)
,	m_start(m_layer->getTime())
,	m_end(m_layer->getTime() + m_layer->getDuration())
{
}

void EffectLayerInstance::update(EmitterUpdateContext& context, const Transform& transform, float time, bool enable)
{
	if (time >= m_start && time <= m_end)
		m_emitterInstance->update(context, transform, enable);
	else if (time > m_end)
		m_emitterInstance->update(context, transform, false);
}

void EffectLayerInstance::synchronize()
{
	m_emitterInstance->synchronize();
}

void EffectLayerInstance::render(PointRenderer* pointRenderer, const Plane& cameraPlane, float time) const
{
	if (time >= m_start)
		m_emitterInstance->render(pointRenderer, cameraPlane);
}

const Aabb& EffectLayerInstance::getBoundingBox() const
{
	return m_emitterInstance->getBoundingBox();
}

	}
}
