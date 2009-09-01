#include <ctime>
#include "Spray/EffectEntity.h"
#include "Spray/Effect.h"
#include "Spray/EffectInstance.h"
#include "Resource/IResourceManager.h"
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

EffectEntity::EffectEntity(resource::IResourceManager* resourceManager, const Transform& transform, const resource::Proxy< Effect >& effect)
:	m_resourceManager(resourceManager)
,	m_transform(transform)
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

void EffectEntity::setTransform(const Transform& transform)
{
	m_transform = transform;
}

bool EffectEntity::getTransform(Transform& outTransform) const
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

	return worldBoundingBox.transform(m_transform.inverse());
}

Aabb EffectEntity::getWorldBoundingBox() const
{
	return m_effectInstance ? m_effectInstance->getBoundingBox() : Aabb();
}

void EffectEntity::update(const world::EntityUpdate* update)
{
	if (!m_effect.valid() || !m_effectInstance)
	{
		if (!m_resourceManager->bind(m_effect))
			return;

		if (!m_effect.validate())
			return;

		m_effectInstance = m_effect->createInstance(m_resourceManager);
	}

	m_context.deltaTime = update->getDeltaTime();
	m_context.deltaTime = min(m_context.deltaTime, c_maxDeltaTime);

	if (m_effectInstance)
		m_effectInstance->update(m_context, m_transform, m_enable);
}

	}
}
