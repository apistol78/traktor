#include "Mesh/Blend/BlendMeshEntity.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.BlendMeshEntity", BlendMeshEntity, MeshEntity)

BlendMeshEntity::BlendMeshEntity(const Transform& transform, bool screenSpaceCulling, const resource::Proxy< BlendMesh >& mesh)
:	MeshEntity(transform, screenSpaceCulling)
,	m_mesh(mesh)
{
}

void BlendMeshEntity::setBlendWeights(const std::vector< float >& blendWeights)
{
	size_t copy = std::min(m_blendWeights.size(), blendWeights.size());
	for (size_t i = 0; i < copy; ++i)
		m_blendWeights[i] = blendWeights[i];
}

const std::vector< float >& BlendMeshEntity::getBlendWeights() const
{
	return m_blendWeights;
}

Aabb3 BlendMeshEntity::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

bool BlendMeshEntity::supportTechnique(render::handle_t technique) const
{
	return m_mesh->supportTechnique(technique);
}

void BlendMeshEntity::precull(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView
)
{
}

void BlendMeshEntity::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	float distance
)
{
	// \fixme Instance needs to be created if mesh has been reloaded.
	if (!m_instance)
	{
		m_instance = m_mesh->createInstance();
		if (!m_instance)
			return;
	}

	uint32_t blendTargetCount = m_mesh->getBlendTargetCount();
	if (blendTargetCount != m_blendWeights.size())
		m_blendWeights.resize(blendTargetCount, 0.0f);

	m_mesh->render(
		worldContext.getRenderContext(),
		worldRenderPass,
		getTransform(worldRenderView.getInterval()),
		m_instance,
		m_blendWeights,
		distance,
		getParameterCallback()
	);
}

	}
}
