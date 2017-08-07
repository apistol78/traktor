/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Mesh/MeshCulling.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshEntity.h"
#include "Mesh/Instance/InstanceMeshEntityRenderer.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.InstanceMeshEntityRenderer", InstanceMeshEntityRenderer, world::IEntityRenderer)

const TypeInfoSet InstanceMeshEntityRenderer::getRenderableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< InstanceMeshEntity >());
	return typeSet;
}

void InstanceMeshEntityRenderer::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	InstanceMeshEntity* meshEntity = checked_type_cast< InstanceMeshEntity* >(renderable);
	T_ASSERT_M (!meshEntity->getParameterCallback(), L"Instance mesh entities doesn't support parameter callback");
	
	InstanceMesh* mesh = meshEntity->m_mesh;

	if (!mesh->supportTechnique(worldRenderPass.getTechnique()))
		return;

	Aabb3 boundingBox = meshEntity->getBoundingBox();
	Transform transform = meshEntity->m_transform.get(worldRenderView.getInterval());

	float distance = 0.0f;
	if (!isMeshVisible(
		boundingBox,
		worldRenderView.getCullFrustum(),
		worldRenderView.getView() * transform.toMatrix44(),
		worldRenderView.getProjection(),
		1e-4f,
		distance
	))
		return;

	Transform transformLast = meshEntity->m_transform.get(0);

	m_meshInstances[mesh].push_back(InstanceMesh::RenderInstance(
		packInstanceMeshData(transform),
		packInstanceMeshData(transformLast),
		distance
	));

	if ((worldRenderPass.getPassFlags() & world::IWorldRenderPass::PfLast) != 0)
		meshEntity->m_transform.step();
}

void InstanceMeshEntityRenderer::flush(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
	for (SmallMap< InstanceMesh*, AlignedVector< InstanceMesh::RenderInstance > >::iterator i = m_meshInstances.begin(); i != m_meshInstances.end(); ++i)
	{
		if (i->second.empty())
			continue;

		T_ASSERT (i->first);
		i->first->render(
			worldContext.getRenderContext(),
			worldRenderPass,
			i->second,
			0
		);

		i->second.resize(0);
	}
}

	}
}
