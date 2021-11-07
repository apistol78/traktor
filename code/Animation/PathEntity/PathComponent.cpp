#include "Animation/PathEntity/PathComponent.h"
#include "Core/Math/Const.h"
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
:	m_owner(nullptr)
,	m_transform(Transform::identity())
,	m_path(path)
,	m_timeMode(timeMode)
,	m_timeOffset(timeOffset)
{
}

void PathComponent::destroy()
{
	m_owner = nullptr;
}

void PathComponent::setOwner(world::Entity* owner)
{
	if ((m_owner = owner) != nullptr)
		m_transform = m_owner->getTransform();
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

	if (update.deltaTime <= FUZZY_EPSILON)
	{
		m_owner->setTransform(m_transform);
		return;
	}

	float time = m_timeOffset;
	if (m_timeMode != TmManual)
	{
		time = update.totalTime + m_timeOffset;
		switch (m_timeMode)
		{
		case TmOnce:
			if (time > m_path.getEndTime())
				time = m_path.getEndTime();
			break;

		case TmLoop:
			time = std::fmod(time, m_path.getEndTime());
			break;

		case TmPingPong:
			{
				int32_t n = (int32_t)(time / m_path.getEndTime());
				time = std::fmod(time, m_path.getEndTime());
				if ((n & 1) == 1)
					time = m_path.getEndTime() - time;
			}
			break;

		default:
			break;
		}
	}

	Transform transform = m_path.evaluate(time, m_timeMode == TmLoop).transform();
	m_owner->setTransform(m_transform * transform);
}

	}
}
