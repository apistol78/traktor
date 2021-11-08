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
	TimeMode timeMode
)
:	m_owner(nullptr)
,	m_transform(Transform::identity())
,	m_path(path)
,	m_timeMode(timeMode)
,	m_time(0.0f)
,	m_timeTarget(0.0f)
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

	if (m_timeMode != TimeMode::Manual)
	{
		m_time = update.totalTime;
		switch (m_timeMode)
		{
		case TimeMode::Once:
			if (m_time > m_path.getEndTime())
				m_time = m_path.getEndTime();
			break;

		case TimeMode::Loop:
			m_time = std::fmod(m_time, m_path.getEndTime());
			break;

		case TimeMode::PingPong:
			{
				int32_t n = (int32_t)(m_time / m_path.getEndTime());
				m_time = std::fmod(m_time, m_path.getEndTime());
				if ((n & 1) == 1)
					m_time = m_path.getEndTime() - m_time;
			}
			break;

		default:
			break;
		}
	}
	else
	{
		if (m_time < m_timeTarget)
			m_time = std::min(m_time + update.deltaTime, m_timeTarget);
		else if (m_time > m_timeTarget)
			m_time = std::max(m_time - update.deltaTime, m_timeTarget);
	}

	Transform transform = m_path.evaluate(m_time, m_timeMode == TimeMode::Loop).transform();
	m_owner->setTransform(m_transform * transform);
}

void PathComponent::continueTo(float time)
{
	m_timeTarget = time;
	m_timeMode = TimeMode::Manual;
}

	}
}
