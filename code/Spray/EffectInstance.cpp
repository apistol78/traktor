#include "Spray/EffectInstance.h"
#include "Spray/Effect.h"
#include "Spray/EffectLayer.h"
#include "Spray/EffectLayerInstance.h"
#include "Spray/Types.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectInstance", EffectInstance, Object)

EffectInstance::EffectInstance(const Effect* effect)
:	m_effect(effect)
,	m_time(0.0f)
{
	m_loopEnable = effect->getLoopEnd() > effect->getLoopStart();
}

void EffectInstance::update(Context& context, const Transform& transform, bool enable)
{
	m_time += context.deltaTime;
	if (m_loopEnable)
	{
		while (m_time >= m_effect->getLoopEnd())
			m_time = m_effect->getLoopStart();
	}

	m_boundingBox = Aabb3();
	for (RefArray< EffectLayerInstance >::iterator i = m_layerInstances.begin(); i != m_layerInstances.end(); ++i)
	{
		(*i)->update(context, transform, m_time, enable);
		m_boundingBox.contain((*i)->getBoundingBox());
	}
}

void EffectInstance::synchronize()
{
	for (RefArray< EffectLayerInstance >::iterator i = m_layerInstances.begin(); i != m_layerInstances.end(); ++i)
		(*i)->synchronize();
}

void EffectInstance::render(
	render::handle_t technique,
	PointRenderer* pointRenderer,
	MeshRenderer* meshRenderer,
	TrailRenderer* trailRenderer,
	const Transform& transform,
	const Vector4& cameraPosition,
	const Plane& cameraPlane
) const
{
	for (RefArray< EffectLayerInstance >::const_iterator i = m_layerInstances.begin(); i != m_layerInstances.end(); ++i)
		(*i)->render(technique, pointRenderer, meshRenderer, trailRenderer, transform, cameraPosition, cameraPlane, m_time);
}

	}
}
