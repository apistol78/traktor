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

class Entity;
class EntityData;
class IEntityEvent;
class IEntityEventData;
class IEntityFactory;

/*! \brief Entity builder interface.
 * \ingroup World
 */
class T_DLLCLASS IEntityBuilder : public Object
{
	T_RTTI_CLASS;

public:
	virtual void addFactory(const IEntityFactory* entityFactory) = 0;

	virtual void removeFactory(const IEntityFactory* entityFactory) = 0;

	virtual const IEntityFactory* getFactory(const EntityData* entityData) const = 0;

	virtual const IEntityFactory* getFactory(const IEntityEventData* entityEventData) const = 0;

	virtual Ref< Entity > create(const EntityData* entityData) const = 0;

	virtual Ref< IEntityEvent > create(const IEntityEventData* entityEventData) const = 0;

	virtual const IEntityBuilder* getCompositeEntityBuilder() const = 0;

	template < typename EntityType >
	Ref< EntityType > create(const EntityData* entityData) const
	{
		return checked_type_cast< EntityType*, true >(create(entityData));
	}

	template < typename EntityEventType >
	Ref< EntityEventType > create(const IEntityEventData* entityEventData) const
	{
		return checked_type_cast< EntityEventType*, true >(create(entityEventData));
	}
};

	}
}

#endif	// traktor_world_IEntityBuilder_H
