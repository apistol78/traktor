#include "Mesh/Skinned/SkinnedMeshEntity.h"
#include "Mesh/Skinned/SkinnedMesh.h"
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
	return validate() ? m_mesh->getBoundingBox() : Aabb3();
}

bool SkinnedMeshEntity::supportTechnique(render::handle_t technique) const
{
	return m_mesh.validate() ? m_mesh->supportTechnique(technique) : false;
}

void SkinnedMeshEntity::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	float distance
)
{
	if (!validate())
		return;

	m_mesh->render(
		worldContext.getRenderContext(),
		worldRenderPass,
		getTransform(worldRenderView.getInterval()),
		m_boneTransforms,
		distance,
		getParameterCallback()
	);
}

bool SkinnedMeshEntity::validate() const
{
	if (m_mesh.valid())
		return true;

	if (!m_mesh.validate())
		return false;

	const std::map< std::wstring, int >& boneMap = m_mesh->getBoneMap();

	m_boneTransforms.resize(boneMap.size() * 2);
	for (uint32_t i = 0; i < uint32_t(m_boneTransforms.size()); ++i)
		m_boneTransforms[i] = Vector4::origo();

	return true;
}

	}
}
