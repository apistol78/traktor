#ifndef traktor_world_EntitySetBuilder_H
#define traktor_world_EntitySetBuilder_H

#include <list>
#include <map>
#include <stack>
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
	EntityBuilder();

	virtual void addFactory(IEntityFactory* entityFactory);

	virtual void removeFactory(IEntityFactory* entityFactory);

	virtual void begin(IEntitySchema* entitySchema);

	virtual Ref< Entity > create(const EntityData* entityData);

	virtual Ref< Entity > get(const EntityData* entityData) const;

	virtual void end();

private:
	typedef std::list< std::pair< std::wstring, Ref< Entity > > > scope_t;

	Semaphore m_lock;
	Ref< IEntitySchema > m_entitySchema;
	RefArray< IEntityFactory > m_entityFactories;
	std::stack< scope_t > m_entityScope;
	std::map< const EntityData*, Ref< Entity > > m_entities;
	std::map< const TypeInfo*, IEntityFactory* > m_resolvedFactoryCache;
};

	}
}

#endif	// traktor_world_EntitySetBuilder_H
