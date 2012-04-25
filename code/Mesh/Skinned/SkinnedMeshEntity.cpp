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

void SkinnedMeshEntity::setBoneTransforms(const AlignedVector< Matrix44 >& boneTransforms)
{
	uint32_t size = uint32_t(m_boneTransforms.size());
	for (uint32_t i = 0, j = 0; i < size; i += 2, ++j)
	{
		if (j < boneTransforms.size())
		{
			Transform bone(boneTransforms[i]);
			m_boneTransforms[i + 0] = bone.rotation().e;
			m_boneTransforms[i + 1] = bone.translation().xyz1();
		}
		else
		{
			m_boneTransforms[i + 0] = Vector4::origo();
			m_boneTransforms[i + 1] = Vector4::origo();
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
	const std::map< std::wstring, int >& boneMap = m_mesh->getBoneMap();
	if (m_boneTransforms.size() < boneMap.size() * 2)
	{
		m_boneTransforms.resize(boneMap.size() * 2);
		for (uint32_t i = 0; i < uint32_t(m_boneTransforms.size()); ++i)
			m_boneTransforms[i] = Vector4::origo();
	}

	m_mesh->render(
		worldContext.getRenderContext(),
		worldRenderPass,
		getTransform(worldRenderView.getInterval()),
		m_boneTransforms,
		distance,
		getParameterCallback()
	);
}

	}
}
