/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Mesh/MeshCulling.h"
#include "Mesh/Blend/BlendMesh.h"
#include "Mesh/Blend/BlendMeshComponent.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.BlendMeshComponent", BlendMeshComponent, MeshComponent)

BlendMeshComponent::BlendMeshComponent(const resource::Proxy< BlendMesh >& mesh)
:	m_mesh(mesh)
{
	uint32_t blendTargetCount = m_mesh->getBlendTargetCount();
	m_blendWeights.resize(blendTargetCount, 0.0f);
}

void BlendMeshComponent::destroy()
{
	m_mesh.clear();
	m_blendWeights.clear();
	MeshComponent::destroy();
}

Aabb3 BlendMeshComponent::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

void BlendMeshComponent::build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass)
{
	if (!m_mesh->supportTechnique(worldRenderPass.getTechnique()))
		return;

	float distance = 0.0f;
	if (!isMeshVisible(
		m_mesh->getBoundingBox(),
		worldRenderView.getCullFrustum(),
		worldRenderView.getView() * m_transform.get().toMatrix44(),
		worldRenderView.getProjection(),
		0.001f,
		distance
	))
		return;

	if (!m_instance || m_mesh.changed())
	{
		m_instance = m_mesh->createInstance();
		if (!m_instance)
			return;
		m_mesh.consume();
	}

	const uint32_t blendTargetCount = m_mesh->getBlendTargetCount();
	if (blendTargetCount != m_blendWeights.size())
		m_blendWeights.resize(blendTargetCount, 0.0f);

	m_mesh->build(
		context.getRenderContext(),
		worldRenderPass,
		m_transform.get0(),
		m_transform.get(worldRenderView.getInterval()),
		m_instance,
		m_blendWeights,
		distance,
		m_parameterCallback
	);
}

void BlendMeshComponent::setBlendWeights(const AlignedVector< float >& blendWeights)
{
	size_t copy = std::min(m_blendWeights.size(), blendWeights.size());
	for (size_t i = 0; i < copy; ++i)
		m_blendWeights[i] = blendWeights[i];
}

const AlignedVector< float >& BlendMeshComponent::getBlendWeights() const
{
	return m_blendWeights;
}

}
