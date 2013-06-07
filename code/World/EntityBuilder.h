#ifndef traktor_world_EntityBuilder_H
#define traktor_world_EntityBuilder_H

#include <map>
#include "Core/RefArray.h"
#include "Core/Thread/Semaphore.h"
#include "World/IEntityBuilder.h"

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

/*! \brief Entity builder.
 * \ingroup World
 */
class T_DLLCLASS EntityBuilder : public IEntityBuilder
{
	T_RTTI_CLASS;

public:
	virtual void addFactory(const IEntityFactory* entityFactory);

	virtual void removeFactory(const IEntityFactory* entityFactory);

	virtual const IEntityFactory* getFactory(const EntityData* entityData) const;

	virtual const IEntityFactory* getFactory(const IEntityEventData* entityEventData) const;

	virtual Ref< Entity > create(const EntityData* entityData) const;

	virtual Ref< IEntityEvent > create(const IEntityEventData* entityEventData) const;

	virtual const IEntityBuilder* getCompositeEntityBuilder() const;

private:
	mutable Semaphore m_lock;
	RefArray< const IEntityFactory > m_entityFactories;
	mutable std::map< const TypeInfo*, const IEntityFactory* > m_resolvedFactoryCache;
};

	}
}

#endif	// traktor_world_EntityBuilder_H
