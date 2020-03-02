#include "Shape/Spline/ControlPointComponent.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.ControlPointComponent", ControlPointComponent, world::IEntityComponent)

ControlPointComponent::ControlPointComponent()
:	m_dirty(true)
,	m_transform(Transform::identity())
{
}

void ControlPointComponent::destroy()
{
}

void ControlPointComponent::setOwner(world::Entity* owner)
{
}

void ControlPointComponent::setTransform(const Transform& transform)
{
	m_dirty = true;
	m_transform = transform;
}

Aabb3 ControlPointComponent::getBoundingBox() const
{
	Aabb3 boundingBox(
		Vector4(-1.0f, -1.0f, -1.0f),
		Vector4( 1.0f,  1.0f,  1.0f)
	);
	return boundingBox;
}

void ControlPointComponent::update(const world::UpdateParams& update)
{
}

bool ControlPointComponent::checkDirty()
{
	bool dirty = m_dirty;
	m_dirty = false;
	return dirty;
}

	}
}
