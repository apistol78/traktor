#ifndef traktor_mesh_SkinnedMeshEntity_H
#define traktor_mesh_SkinnedMeshEntity_H

#include "Core/Containers/AlignedVector.h"
#include "Mesh/MeshEntity.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class SkinnedMesh;

class T_DLLCLASS SkinnedMeshEntity : public MeshEntity
{
	T_RTTI_CLASS;

public:
	SkinnedMeshEntity(const Transform& transform, const resource::Proxy< SkinnedMesh >& mesh);

	void setBoneTransforms(const AlignedVector< Matrix44 >& boneTransforms);

	virtual Aabb3 getBoundingBox() const;

	virtual bool supportTechnique(render::handle_t technique) const;

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		float distance
	);

private:
	mutable resource::Proxy< SkinnedMesh > m_mesh;
	mutable AlignedVector< Vector4 > m_boneTransforms;

	bool validate() const;
};

	}
}

#endif	// traktor_mesh_SkinnedMeshEntity_H
