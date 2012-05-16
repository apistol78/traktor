#include "Mesh/Skinned/SkinnedMesh.h"
#include "Mesh/Skinned/SkinnedMeshEntity.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.SkinnedMeshEntity", SkinnedMeshEntity, MeshEntity)

SkinnedMeshEntity::SkinnedMeshEntity(const Transform& transform, const resource::Proxy< SkinnedMesh >& mesh)
:	MeshEntity(transform)
,	m_mesh(mesh)
{
}

void SkinnedMeshEntity::setJointTransforms(const AlignedVector< Matrix44 >& jointTransforms)
{
	uint32_t size = uint32_t(m_jointTransforms.size());
	for (uint32_t i = 0, j = 0; i < size; i += 2, ++j)
	{
		if (j < jointTransforms.size())
		{
			Transform joint(jointTransforms[i]);
			m_jointTransforms[i + 0] = joint.rotation().e;
			m_jointTransforms[i + 1] = joint.translation().xyz1();
		}
		else
		{
			m_jointTransforms[i + 0] = Vector4::origo();
			m_jointTransforms[i + 1] = Vector4::origo();
		}
	}
}

Aabb3 SkinnedMeshEntity::getBoundingBox() const
{
	return m_mesh->getBoundingBox();
}

bool SkinnedMeshEntity::supportTechnique(render::handle_t technique) const
{
	return m_mesh->supportTechnique(technique);
}

void SkinnedMeshEntity::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	float distance
)
{
	// Ensure bone transform array is at least enough size.
	const std::map< std::wstring, int >& jointMap = m_mesh->getJointMap();
	if (m_jointTransforms.size() < jointMap.size() * 2)
	{
		m_jointTransforms.resize(jointMap.size() * 2);
		for (uint32_t i = 0; i < uint32_t(m_jointTransforms.size()); ++i)
			m_jointTransforms[i] = Vector4::origo();
	}

	m_mesh->render(
		worldContext.getRenderContext(),
		worldRenderPass,
		getTransform(worldRenderView.getInterval()),
		m_jointTransforms,
		distance,
		getParameterCallback()
	);
}

	}
}
