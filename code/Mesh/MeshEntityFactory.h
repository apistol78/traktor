#ifndef traktor_mesh_MeshEntityFactory_H
#define traktor_mesh_MeshEntityFactory_H

#include "World/Entity/EntityFactory.h"

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

class T_DLLCLASS MeshEntityFactory : public world::EntityFactory
{
	T_RTTI_CLASS(MeshEntityFactory)

public:
	virtual const TypeSet getEntityTypes() const;

	virtual world::Entity* createEntity(world::EntityBuilder* builder, const world::EntityData& entityData) const;
};

	}
}

#endif	// traktor_mesh_MeshEntityFactory_H
