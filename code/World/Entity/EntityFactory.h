#ifndef traktor_world_EntityFactory_H
#define traktor_world_EntityFactory_H

#include <vector>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class Entity;
class EntityData;
class EntityBuilder;

/*! \brief Entity factory.
 * \ingroup World
 */
class T_DLLCLASS EntityFactory : public Object
{
	T_RTTI_CLASS(EntityFactory)

public:
	virtual const TypeSet getEntityTypes() const = 0;

	virtual Entity* createEntity(EntityBuilder* builder, const EntityData& entityData) const = 0;
};

	}
}

#endif	// traktor_world_EntityFactory_H
