#ifndef traktor_mesh_MeshEntityFactory_H
#define traktor_mesh_MeshEntityFactory_H

#include "World/Entity/IEntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace mesh
	{

class T_DLLCLASS MeshEntityFactory : public world::IEntityFactory
{
	T_RTTI_CLASS;

public:
	MeshEntityFactory(resource::IResourceManager* resourceManager);

	virtual const TypeInfoSet getEntityTypes() const;

	virtual Ref< world::Entity > createEntity(world::IEntityBuilder* builder, const world::EntityData& entityData) const;

private:
	Ref< resource::IResourceManager > m_resourceManager;
};

	}
}

#endif	// traktor_mesh_MeshEntityFactory_H
