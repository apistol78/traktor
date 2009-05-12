#ifndef traktor_mesh_CompositeMeshEntity_H
#define traktor_mesh_CompositeMeshEntity_H

#include <map>
#include "Core/Heap/Ref.h"
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

class T_DLLCLASS CompositeMeshEntity : public MeshEntity
{
	T_RTTI_CLASS(CompositeMeshEntity)

public:
	CompositeMeshEntity(const Matrix44& transform);
	
	virtual void setTransform(const Matrix44& transform);

	virtual Aabb getBoundingBox() const;

	virtual void render(world::WorldContext* worldContext, world::WorldRenderView* worldRenderView, float distance);

	virtual void update(const world::EntityUpdate* update);

	void addMeshEntity(const std::wstring& name, MeshEntity* meshEntity);

	MeshEntity* getMeshEntity(const std::wstring& name) const;

	inline const std::map< std::wstring, Ref< MeshEntity > >& getMeshEntities() const { return m_meshEntities; }

private:
	std::map< std::wstring, Ref< MeshEntity > > m_meshEntities;
};

	}
}

#endif	// traktor_mesh_CompositeMeshEntity_H
