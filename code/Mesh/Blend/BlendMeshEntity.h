#ifndef traktor_mesh_BlendMeshEntity_H
#define traktor_mesh_BlendMeshEntity_H

#include "Resource/Proxy.h"
#include "Mesh/MeshEntity.h"
#include "Mesh/Blend/BlendMesh.h"

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

/*! \brief Blend mesh entity
 * \ingroup Mesh
 */
class T_DLLCLASS BlendMeshEntity : public MeshEntity
{
	T_RTTI_CLASS;

public:
	BlendMeshEntity(const Transform& transform, const resource::Proxy< BlendMesh >& mesh);

	void setBlendWeights(const std::vector< float >& blendWeights);

	const std::vector< float >& getBlendWeights() const;

	virtual Aabb getBoundingBox() const;

	virtual void render(world::WorldContext* worldContext, world::WorldRenderView* worldRenderView, float distance);

	virtual void update(const world::EntityUpdate* update);

private:
	mutable resource::Proxy< BlendMesh > m_mesh;
	mutable Ref< BlendMesh::Instance > m_instance;
	mutable std::vector< float > m_blendWeights;

	bool validate() const;
};

	}
}

#endif	// traktor_mesh_BlendMeshEntity_H
