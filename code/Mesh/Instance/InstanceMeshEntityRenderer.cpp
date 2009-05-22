#include "Mesh/Instance/InstanceMeshEntityRenderer.h"
#include "Mesh/Instance/InstanceMeshEntity.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/MeshCulling.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.InstanceMeshEntityRenderer", InstanceMeshEntityRenderer, world::EntityRenderer)

const TypeSet InstanceMeshEntityRenderer::getEntityTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< InstanceMeshEntity >());
	return typeSet;
}

void InstanceMeshEntityRenderer::render(
	world::WorldContext* worldContext,
	world::WorldRenderView* worldRenderView,
	world::Entity* entity
)
{
	InstanceMeshEntity* meshEntity = checked_type_cast< InstanceMeshEntity* >(entity);
	T_ASSERT_M (!meshEntity->getParameterCallback(), L"Instance mesh entities doesn't support parameter callback");
	
	resource::Proxy< InstanceMesh >& mesh = meshEntity->getMesh();
	if (!mesh.validate())
		return;

	Aabb boundingBox = meshEntity->getBoundingBox();
	Matrix44 transform; meshEntity->getTransform(transform);

	float distance;
	if (!isMeshVisible(
		boundingBox,
		worldRenderView->getCullFrustum(),
		transform * worldRenderView->getView(),
		worldRenderView->getProjection(),
		1e-4f,
		distance
	))
		return;

	m_meshInstances[mesh].push_back(packInstanceMeshData(transform, meshEntity->getUserParameter()));
}

void InstanceMeshEntityRenderer::flush(
	world::WorldContext* worldContext,
	world::WorldRenderView* worldRenderView
)
{
	for (std::map< InstanceMesh*, AlignedVector< InstanceMeshData > >::iterator i = m_meshInstances.begin(); i != m_meshInstances.end(); ++i)
	{
		if (i->second.empty())
			continue;

		i->first->render(
			worldContext->getRenderContext(),
			worldRenderView,
			i->second
		);

		i->second.resize(0);
	}

	m_meshInstances.clear();
}

	}
}
