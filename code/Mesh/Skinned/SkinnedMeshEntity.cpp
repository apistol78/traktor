#include "Core/Thread/Atomic.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Mesh/Skinned/SkinnedMeshEntity.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.SkinnedMeshEntity", SkinnedMeshEntity, MeshEntity)

SkinnedMeshEntity::SkinnedMeshEntity(const Transform& transform, bool screenSpaceCulling, const resource::Proxy< SkinnedMesh >& mesh)
:	MeshEntity(transform, screenSpaceCulling)
,	m_mesh(mesh)
,	m_count(0)
{
	const std::map< std::wstring, int >& jointMap = m_mesh->getJointMap();
	m_jointTransforms[0].resize(jointMap.size() * 2, Vector4::origo());
	m_jointTransforms[1].resize(jointMap.size() * 2, Vector4::origo());
}

void SkinnedMeshEntity::setJointTransforms(const AlignedVector< Matrix44 >& jointTransforms_)
{
	AlignedVector< Vector4 >& jointTransforms = m_jointTransforms[m_count];

	const std::map< std::wstring, int >& jointMap = m_mesh->getJointMap();
	jointTransforms.resize(jointMap.size() * 2, Vector4::origo());

	uint32_t size = uint32_t(jointTransforms_.size());
	for (uint32_t i = 0, j = 0; i < size; i += 2, ++j)
	{
		if (j < jointTransforms_.size())
		{
			Transform joint(jointTransforms_[i]);
			jointTransforms[i + 0] = joint.rotation().e;
			jointTransforms[i + 1] = joint.translation().xyz1();
		}
		else
		{
			jointTransforms[i + 0] = Vector4::origo();
			jointTransforms[i + 1] = Vector4::origo();
		}
	}

	Atomic::exchange(m_count, 1 - m_count);
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
	const AlignedVector< Vector4 >& jointTransforms = m_jointTransforms[m_count];
	m_mesh->render(
		worldContext.getRenderContext(),
		worldRenderPass,
		m_transform.get(worldRenderView.getInterval()),
		jointTransforms,
		distance,
		getParameterCallback()
	);
}

	}
}
