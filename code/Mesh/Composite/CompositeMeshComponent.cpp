/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/Composite/CompositeMeshComponent.h"
#include "World/WorldRenderView.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.CompositeMeshComponent", CompositeMeshComponent, MeshComponent)

void CompositeMeshComponent::destroy()
{
	m_meshComponents.clear();
	MeshComponent::destroy();
}

Aabb3 CompositeMeshComponent::getBoundingBox() const
{
	Transform invTransform = m_transform.get().inverse();

	Aabb3 boundingBox;
	for (auto meshComponent : m_meshComponents)
	{
		const Aabb3 childBoundingBox = meshComponent->getBoundingBox();
		if (!childBoundingBox.empty())
		{
			const Transform childTransform = meshComponent->getTransform().get();
			const Transform intoParentTransform = invTransform * childTransform;
			boundingBox.contain(childBoundingBox.transform(intoParentTransform));
		}
	}

	return boundingBox;
}

void CompositeMeshComponent::setTransform(const Transform& transform)
{
	Transform invTransform = m_transform.get().inverse();
	for (auto meshComponent : m_meshComponents)
	{
		const Transform currentTransform = meshComponent->getTransform().get();
		const Transform Tlocal = invTransform * currentTransform;
		const Transform Tworld = transform * Tlocal;
		meshComponent->setTransform(Tworld);
	}
	MeshComponent::setTransform(transform);
}

void CompositeMeshComponent::update(const world::UpdateParams& update)
{
	for (auto meshComponent : m_meshComponents)
		meshComponent->update(update);
	MeshComponent::update(update);
}

void CompositeMeshComponent::build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass)
{
	for (auto meshComponent : m_meshComponents)
		meshComponent->build(context, worldRenderView, worldRenderPass);
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
