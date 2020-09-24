#include "Animation/Rotator/RotatorComponent.h"
#include "World/Entity.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.RotatorComponent", RotatorComponent, world::IEntityComponent)

RotatorComponent::RotatorComponent(Axis axis, float rate)
:	m_axis(axis)
,	m_rate(rate)
{
}

void RotatorComponent::destroy()
{
	m_owner = nullptr;
}

void RotatorComponent::setOwner(world::Entity* owner)
{
	if ((m_owner = owner) != nullptr)
		m_transform = m_owner->getTransform();
}

void RotatorComponent::setTransform(const Transform& transform)
{
	m_transform = transform;
}

Aabb3 RotatorComponent::getBoundingBox() const
{
	return Aabb3();
}

void RotatorComponent::update(const world::UpdateParams& update)
{
	if (!m_owner)
		return;

	m_angle += m_rate * update.deltaTime;

	Transform transform;
	switch (m_axis)
	{
	case Axis::X:
		transform = Transform(rotateX(m_angle));
		break;

	case Axis::Y:
		transform = Transform(rotateY(m_angle));
		break;

	case Axis::Z:
		transform = Transform(rotateZ(m_angle));
		break;

	default:
		transform = Transform::identity();
		break;
	}

	m_owner->setTransform(m_transform * transform);
}

	}
}
