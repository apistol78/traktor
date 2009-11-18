#ifndef traktor_mesh_CompositeMeshEntityData_H
#define traktor_mesh_CompositeMeshEntityData_H

#include "Core/RefArray.h"
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
	namespace world
	{

class EntityInstance;

	}

	namespace mesh
	{

class T_DLLCLASS CompositeMeshEntityData : public MeshEntityData
{
	T_RTTI_CLASS;

public:
	virtual Ref< MeshEntity > createEntity(resource::IResourceManager* resourceManager, world::IEntityBuilder* builder) const;

	virtual bool serialize(ISerializer& s);

	inline const RefArray< world::EntityInstance >& getInstances() const { return m_instances; }

private:
	RefArray< world::EntityInstance > m_instances;
};

	}
}

#endif	// traktor_mesh_CompositeMeshEntityData_H
