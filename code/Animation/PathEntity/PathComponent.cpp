/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Animation/PathEntity/PathComponent.h"
#include "World/Entity.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.PathComponent", PathComponent, world::IEntityComponent)

PathComponent::PathComponent(
	const TransformPath& path,
	TimeMode timeMode,
	float timeOffset
)
:	m_owner(0)
,	m_transform(Transform::identity())
,	m_path(path)
,	m_timeMode(timeMode)
,	m_timeScale(1.0f)
,	m_timeDeltaSign(1.0f)
,	m_time(timeOffset)
{
}

void PathComponent::destroy()
{
	m_owner = 0;
}

void PathComponent::setOwner(world::Entity* owner)
{
	if ((m_owner = owner) != 0)
		m_owner->getTransform(m_transform);
}

void PathComponent::setTransform(const Transform& transform)
{
	m_transform = transform;
}

Aabb3 PathComponent::getBoundingBox() const
{
	return Aabb3();
}

void PathComponent::update(const world::UpdateParams& update)
{
	if (!m_owner)
		return;

	if (m_timeMode != TmManual)
	{
		m_time += update.deltaTime * m_timeDeltaSign;

		switch (m_timeMode)
		{
		case TmOnce:
			if (m_time > m_path.getEndTime())
			{
				//if (m_listener && abs(m_timeDeltaSign) > 0.0f)
				//	m_listener->notifyPathFinished(this);

				m_time = m_path.getEndTime();
				m_timeDeltaSign = 0.0f;
			}
			break;

		case TmLoop:
			if (m_time > m_path.getEndTime())
			{
				//if (m_listener)
				//	m_listener->notifyPathFinished(this);

				m_time = 0.0f;
			}
			break;

		case TmPingPong:
			if (m_time > m_path.getEndTime())
			{
				//if (m_listener)
				//	m_listener->notifyPathFinished(this);

				m_time = m_path.getEndTime();
				m_timeDeltaSign = -1.0f;
			}
			else if (m_time < 0.0f)
			{
				//if (m_listener)
				//	m_listener->notifyPathFinished(this);

				m_time = 0.0f;
				m_timeDeltaSign = 1.0f;
			}
			break;

		default:
			break;
		}
	}

	Transform transform = m_path.evaluate(m_time, m_timeMode == TmLoop).transform();
	m_owner->setTransform(m_transform * transform);
}

	}
}
