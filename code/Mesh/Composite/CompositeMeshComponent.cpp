/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Mesh/Composite/CompositeMeshComponent.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.CompositeMeshComponent", CompositeMeshComponent, MeshComponent)

CompositeMeshComponent::CompositeMeshComponent()
:	MeshComponent(false)
{
}

void CompositeMeshComponent::destroy()
{
	m_meshComponents.clear();
	MeshComponent::destroy();
}

Aabb3 CompositeMeshComponent::getBoundingBox() const
{
	Transform invTransform = m_transform.get().inverse();

	Aabb3 boundingBox;
	for (RefArray< MeshComponent >::const_iterator i = m_meshComponents.begin(); i != m_meshComponents.end(); ++i)
	{
		Aabb3 childBoundingBox = (*i)->getBoundingBox();
		if (!childBoundingBox.empty())
		{
			Transform childTransform = (*i)->getTransform().get();
			Transform intoParentTransform = invTransform * childTransform;
			boundingBox.contain(childBoundingBox.transform(intoParentTransform));
		}
	}

	return boundingBox;
}

void CompositeMeshComponent::setTransform(const Transform& transform)
{
	Transform invTransform = m_transform.get().inverse();
	for (RefArray< MeshComponent >::iterator i = m_meshComponents.begin(); i != m_meshComponents.end(); ++i)
	{
		Transform currentTransform = (*i)->getTransform().get();
		Transform Tlocal = invTransform * currentTransform;
		Transform Tworld = transform * Tlocal;
		(*i)->setTransform(Tworld);
		//(*i)->update({ 0.0f, 0.0f, 0.0f });
	}
	MeshComponent::setTransform(transform);
}

void CompositeMeshComponent::update(const world::UpdateParams& update)
{
	for (RefArray< MeshComponent >::iterator i = m_meshComponents.begin(); i != m_meshComponents.end(); ++i)
		(*i)->update(update);
	MeshComponent::update(update);
}

void CompositeMeshComponent::render(world::WorldContext& worldContext, world::WorldRenderView& worldRenderView, world::IWorldRenderPass& worldRenderPass)
{
	for (RefArray< MeshComponent >::iterator i = m_meshComponents.begin(); i != m_meshComponents.end(); ++i)
		(*i)->render(worldContext, worldRenderView, worldRenderPass);
}

void CompositeMeshComponent::removeAll()
{
	m_meshComponents.resize(0);
}

void CompositeMeshComponent::remove(MeshComponent* meshComponent)
{
	m_meshComponents.remove(meshComponent);
}

void CompositeMeshComponent::add(MeshComponent* meshComponent)
{
	m_meshComponents.push_back(meshComponent);
}

	}
}
