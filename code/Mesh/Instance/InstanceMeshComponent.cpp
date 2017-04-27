/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Mesh/MeshCulling.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshComponent.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.InstanceMeshComponent", InstanceMeshComponent, MeshComponent)

InstanceMeshComponent::InstanceMeshComponent(const resource::Proxy< InstanceMesh >& mesh, bool screenSpaceCulling)
:	MeshComponent(screenSpaceCulling)
,	m_mesh(mesh)
{
}

void InstanceMeshComponent::destroy()
{
	m_mesh.clear();
	MeshComponent::destroy();
}

Aabb3 InstanceMeshComponent::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

void InstanceMeshComponent::render(world::WorldContext& worldContext, world::WorldRenderView& worldRenderView, world::IWorldRenderPass& worldRenderPass)
{
	T_ASSERT_M (0, L"Forgot to register InstanceMeshComponentRenderer?");
}

	}
}
