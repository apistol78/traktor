#ifndef traktor_illuminate_IlluminateEntityFactory_H
#define traktor_illuminate_IlluminateEntityFactory_H

#include "World/IEntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ILLUMINATE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace illuminate
	{

class T_DLLCLASS IlluminateEntityFactory : public world::IEntityFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEntityTypes() const;

	virtual const TypeInfoSet getEntityEventTypes() const;

	virtual Ref< world::Entity > createEntity(const world::IEntityBuilder* builder, const world::EntityData& entityData) const;

	virtual Ref< world::IEntityEvent > createEntityEvent(const world::IEntityBuilder* builder, const world::IEntityEventData& entityEventData) const;
};

	}
}

#endif	// traktor_illuminate_IlluminateEntityFactory_H
