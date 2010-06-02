#include "Animation/PathEntity/PathEntity.h"
#include "World/WorldContext.h"
#include "World/Entity/EntityUpdate.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.PathEntity", PathEntity, world::SpatialEntity)

PathEntity::PathEntity(const Transform& transform, const TransformPath& path, TimeMode timeMode, world::SpatialEntity* entity)
:	m_transform(transform)
,	m_path(path)
,	m_timeMode(timeMode)
,	m_entity(entity)
,	m_timeScale(1.0f)
,	m_timeDeltaSign(1.0f)
,	m_time(0.0f)
{
}

PathEntity::~PathEntity()
{
	destroy();
}

void PathEntity::destroy()
{
	if (m_entity)
	{
		m_entity->destroy();
		m_entity = 0;
	}
}

void PathEntity::setTransform(const Transform& transform)
{
	m_transform = transform;
}

bool PathEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform;
	return true;
}

Aabb PathEntity::getBoundingBox() const
{
	return m_entity ? m_entity->getBoundingBox() : Aabb();
}

void PathEntity::update(const world::EntityUpdate* update)
{
	if (!m_entity)
		return;

	if (m_timeMode != TmManual)
	{
		m_time += update->getDeltaTime() * m_timeDeltaSign;

		switch (m_timeMode)
		{
		case TmOnce:
			if (m_time > m_path.getEndTime())
			{
				m_time = m_path.getEndTime();
				m_timeDeltaSign = 0.0f;
			}
			break;

		case TmLoop:
			if (m_time > m_path.getEndTime())
				m_time = 0.0f;
			break;

		case TmPingPong:
			if (m_time > m_path.getEndTime())
			{
				m_time = m_path.getEndTime();
				m_timeDeltaSign = -1.0f;
			}
			else if (m_time < 0.0f)
			{
				m_time = 0.0f;
				m_timeDeltaSign = 1.0f;
			}
			break;
		}
	}

	Transform transform = m_path.evaluate(m_time, m_timeMode == TmLoop).transform();

	m_entity->setTransform(m_transform * transform);
	m_entity->update(update);
}

void PathEntity::render(world::WorldContext* worldContext, world::WorldRenderView* worldRenderView)
{
	if (m_entity)
		worldContext->build(worldRenderView, m_entity);
}

	}
}
