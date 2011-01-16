#ifndef traktor_mesh_CompositeMeshEntity_H
#define traktor_mesh_CompositeMeshEntity_H

#include <map>
#include "Mesh/MeshEntity.h"

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

class T_DLLCLASS CompositeMeshEntity : public MeshEntity
{
	T_RTTI_CLASS;

public:
	CompositeMeshEntity(const Transform& transform);
	
	virtual void setTransform(const Transform& transform);

	virtual Aabb3 getBoundingBox() const;

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		float distance
	);

	virtual void update(const world::EntityUpdate* update);

	void addMeshEntity(const std::wstring& name, MeshEntity* meshEntity);

	Ref< MeshEntity > getMeshEntity(const std::wstring& name) const;

	inline const std::map< std::wstring, Ref< MeshEntity > >& getMeshEntities() const { return m_meshEntities; }

private:
	std::map< std::wstring, Ref< MeshEntity > > m_meshEntities;
};

	}
}

#endif	// traktor_mesh_CompositeMeshEntity_H
