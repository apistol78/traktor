#ifndef traktor_mesh_MeshEntityData_H
#define traktor_mesh_MeshEntityData_H

#include "World/Entity/SpatialEntityData.h"
#include "Core/Math/Matrix44.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace world
	{

class IEntityBuilder;

	}

	namespace mesh
	{

class MeshEntity;

class T_DLLCLASS MeshEntityData : public world::SpatialEntityData
{
	T_RTTI_CLASS;

public:
	virtual Ref< MeshEntity > createEntity(resource::IResourceManager* resourceManager, world::IEntityBuilder* builder) const = 0;
};

	}
}

#endif	// traktor_mesh_MeshEntityData_H
