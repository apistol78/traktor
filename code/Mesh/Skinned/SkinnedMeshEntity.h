#ifndef traktor_mesh_SkinnedMeshEntity_H
#define traktor_mesh_SkinnedMeshEntity_H

#include "Core/Containers/AlignedVector.h"
#include "Resource/Proxy.h"
#include "Mesh/MeshEntity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class SkinnedMesh;

class T_DLLCLASS SkinnedMeshEntity : public MeshEntity
{
	T_RTTI_CLASS(SkinnedMeshEntity)

public:
	SkinnedMeshEntity(const Transform& transform, const resource::Proxy< SkinnedMesh >& mesh);

	void setBoneTransforms(const AlignedVector< Matrix44 >& boneTransforms);

	const AlignedVector< Matrix44 >& getBoneTransforms() const;

	virtual Aabb getBoundingBox() const;

	virtual void render(world::WorldContext* worldContext, world::WorldRenderView* worldRenderView, float distance);

	virtual void update(const world::EntityUpdate* update);

private:
	mutable resource::Proxy< SkinnedMesh > m_mesh;
	mutable AlignedVector< Matrix44 > m_boneTransforms;

	bool validate() const;
};

	}
}

#endif	// traktor_mesh_SkinnedMeshEntity_H
