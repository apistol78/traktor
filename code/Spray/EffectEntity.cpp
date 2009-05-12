#include <ctime>
#include "Spray/EffectEntity.h"
#include "Spray/Effect.h"
#include "Spray/EffectInstance.h"
#include "World/Entity/EntityUpdate.h"

namespace traktor
{
	namespace spray
	{
		namespace
		{

const float c_maxDeltaTime = 1.0f / 30.0f;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectEntity", EffectEntity, world::SpatialEntity)

EffectEntity::EffectEntity(const Matrix44& transform, const resource::Proxy< Effect >& effect)
:	m_transform(transform)
,	m_effect(effect)
#if !defined(WINCE)
,	m_context(float(clock()))
#endif
,	m_enable(true)
{
}

void EffectEntity::render(const Plane& cameraPlane, PointRenderer* pointRenderer)
{
	if (m_effectInstance)
		m_effectInstance->render(pointRenderer, cameraPlane);
}

void EffectEntity::setTransform(const Matrix44& transform)
{
	m_transform = transform;
}

bool EffectEntity::getTransform(Matrix44& outTransform) const
{
	outTransform = m_transform;
	return true;
}

Aabb EffectEntity::getBoundingBox() const
{
	if (!m_effectInstance)
		return Aabb();

	const Aabb& worldBoundingBox = m_effectInstance->getBoundingBox();
	if (worldBoundingBox.empty())
		return Aabb();

	return worldBoundingBox.transform(m_transform.inverseOrtho());
}

Aabb EffectEntity::getWorldBoundingBox() const
{
	return m_effectInstance ? m_effectInstance->getBoundingBox() : Aabb();
}

void EffectEntity::update(const world::EntityUpdate* update)
{
	if (!m_effect.valid() || !m_effectInstance)
	{
		if (!m_effect.validate())
			return;

		m_effectInstance = m_effect->createInstance();
	}

	m_context.deltaTime = update->getDeltaTime();
	m_context.deltaTime = min(m_context.deltaTime, c_maxDeltaTime);

	if (m_effectInstance)
		m_effectInstance->update(m_context, m_transform, m_enable);
}

	}
}
