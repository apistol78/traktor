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
const uint32_t c_updateDenom = 1;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectEntity", EffectEntity, world::SpatialEntity)

EffectEntity::EffectEntity(resource::IResourceManager* resourceManager, const Transform& transform, const resource::Proxy< Effect >& effect)
:	m_resourceManager(resourceManager)
,	m_transform(transform)
,	m_effect(effect)
#if !defined(WINCE)
,	m_context(0.0f, uint32_t(clock()))
#endif
,	m_counter(0)
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

Aabb3 EffectEntity::getBoundingBox() const
{
	if (!m_effectInstance)
		return Aabb3();

	const Aabb3& worldBoundingBox = m_effectInstance->getBoundingBox();
	if (worldBoundingBox.empty())
		return Aabb3();

	return worldBoundingBox.transform(m_transform.inverse());
}

Aabb3 EffectEntity::getWorldBoundingBox() const
{
	return m_effectInstance ? m_effectInstance->getBoundingBox() : Aabb3();
}

void EffectEntity::update(const world::EntityUpdate* update)
{
	if ((m_counter++ % c_updateDenom) != 0)
		return;

	if (!m_effect.valid() || !m_effectInstance)
	{
		if (!m_effect.valid())
		{
			if (!m_resourceManager->bind(m_effect))
				return;

			if (!m_effect.validate())
				return;
		}

		m_effectInstance = m_effect->createInstance();
	}

	if (m_effectInstance)
	{
		m_context.deltaTime = update->getDeltaTime();
		m_context.deltaTime = min(m_context.deltaTime, c_maxDeltaTime);
		m_context.deltaTime *= c_updateDenom;

		m_effectInstance->update(m_context, m_transform, m_enable);
	}
}

	}
}
