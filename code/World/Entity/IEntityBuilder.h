#ifndef traktor_world_IEntityBuilder_H
#define traktor_world_IEntityBuilder_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class IEntityFactory;
class IEntitySchema;
class EntityData;
class Entity;

/*! \brief Entity builder interface.
 * \ingroup World
 */
class T_DLLCLASS IEntityBuilder : public Object
{
	T_RTTI_CLASS;

public:
	virtual void addFactory(IEntityFactory* entityFactory) = 0;

	virtual void removeFactory(IEntityFactory* entityFactory) = 0;

	virtual void begin(IEntitySchema* entitySchema) = 0;

	virtual Ref< Entity > create(const EntityData* entityData) = 0;

	virtual Ref< Entity > get(const EntityData* entityData) const = 0;

	virtual void end() = 0;

	template < typename EntityType >
	Ref< EntityType > create(const EntityData* entityData)
	{
		return checked_type_cast< EntityType*, true >(create(entityData));
	}
};

	}
}

#endif	// traktor_world_IEntityBuilder_H
