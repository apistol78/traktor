#include "Mesh/MeshCulling.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshComponent.h"
#include "Mesh/Instance/InstanceMeshComponentRenderer.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.InstanceMeshComponentRenderer", InstanceMeshComponentRenderer, world::IEntityRenderer)

const TypeInfoSet InstanceMeshComponentRenderer::getRenderableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< InstanceMeshComponent >());
	return typeSet;
}

void InstanceMeshComponentRenderer::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	InstanceMeshComponent* meshComponent = checked_type_cast< InstanceMeshComponent* >(renderable);
	InstanceMesh* mesh = meshComponent->getMesh();

	if (!mesh->supportTechnique(worldRenderPass.getTechnique()))
		return;

	Aabb3 boundingBox = meshComponent->getBoundingBox();
	Transform transform = meshComponent->getTransform().get(worldRenderView.getInterval());

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

	m_meshInstances[mesh].push_back(std::make_pair(
		packInstanceMeshData(transform),
		distance
	));
}

void InstanceMeshComponentRenderer::flush(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
	for (SmallMap< InstanceMesh*, AlignedVector< InstanceMesh::instance_distance_t > >::iterator i = m_meshInstances.begin(); i != m_meshInstances.end(); ++i)
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
