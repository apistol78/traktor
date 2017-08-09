/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Mesh/MeshComponent.h"
#include "World/Entity.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshComponent", MeshComponent, world::IEntityComponent)

MeshComponent::MeshComponent(bool screenSpaceCulling)
:	m_owner(0)
,	m_screenSpaceCulling(screenSpaceCulling)
,	m_transform(Transform::identity())
{
}

void MeshComponent::destroy()
{
	m_owner = 0;
}

void MeshComponent::setOwner(world::Entity* owner)
{
	if ((m_owner = owner) != 0)
	{
		Transform T;
		if (m_owner->getTransform(T))
			m_transform = IntervalTransform(T);
	}
}

void MeshComponent::setTransform(const Transform& transform)
{
	m_transform.set(transform);
}

void MeshComponent::update(const world::UpdateParams& update)
{
	m_transform.step();
}

	}
}
