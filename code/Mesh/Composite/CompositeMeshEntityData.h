#ifndef traktor_mesh_CompositeMeshEntityData_H
#define traktor_mesh_CompositeMeshEntityData_H

#include "Core/Heap/Ref.h"
#include "Mesh/MeshEntityData.h"

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

class T_DLLCLASS CompositeMeshEntityData : public MeshEntityData
{
	T_RTTI_CLASS(CompositeMeshEntityData)

public:
	virtual MeshEntity* createEntity(world::EntityBuilder* builder) const;

	virtual bool serialize(Serializer& s);

	inline const RefArray< MeshEntityData >& getMeshEntities() const { return m_meshEntities; }

private:
	RefArray< MeshEntityData > m_meshEntities;
};

	}
}

#endif	// traktor_mesh_CompositeMeshEntityData_H
