#ifndef traktor_world_WorldEntityFactory_H
#define traktor_world_WorldEntityFactory_H

#include "World/Entity/IEntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Database;

	}

	namespace world
	{

class ExternalEntityDataCache;

/*! \brief World entity factory.
 * \ingroup World
 */
class T_DLLCLASS WorldEntityFactory : public IEntityFactory
{
	T_RTTI_CLASS;
	
public:
	WorldEntityFactory(db::Database* database, ExternalEntityDataCache* externalCache);

	virtual const TypeInfoSet getEntityTypes() const;

	virtual Ref< Entity > createEntity(IEntityBuilder* builder, const EntityData& entityData) const;

private:
	Ref< db::Database > m_database;
	mutable Ref< ExternalEntityDataCache > m_externalCache;
};
	
	}
}

#endif	// traktor_world_WorldEntityFactory_H
