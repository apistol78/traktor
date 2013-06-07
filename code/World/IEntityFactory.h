#ifndef traktor_world_IEntityFactory_H
#define traktor_world_IEntityFactory_H

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

class IEntityBuilder;
class IEntityEvent;
class IEntityEventData;
class Entity;
class EntityData;

/*! \brief Entity factory interface.
 * \ingroup World
 */
class T_DLLCLASS IEntityFactory : public Object
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEntityTypes() const = 0;

	virtual const TypeInfoSet getEntityEventTypes() const = 0;

	virtual Ref< Entity > createEntity(
		const IEntityBuilder* builder,
		const EntityData& entityData
	) const = 0;

	virtual Ref< IEntityEvent > createEntityEvent(
		const IEntityBuilder* builder,
		const IEntityEventData& entityEventData
	) const = 0;
};

	}
}

#endif	// traktor_world_IEntityFactory_H
