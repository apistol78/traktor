#ifndef traktor_world_ExternalEntityFactory_H
#define traktor_world_ExternalEntityFactory_H

#include "Core/Heap/Ref.h"
#include "World/Entity/EntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class Database;

	}

	namespace world
	{

/*! \brief External entity factory.
 * \ingroup World
 */
class T_DLLCLASS ExternalEntityFactory : public EntityFactory
{
	T_RTTI_CLASS(ExternalEntityFactory)
	
public:
	ExternalEntityFactory(db::Database* database);

	virtual const TypeSet getEntityTypes() const;

	virtual Entity* createEntity(EntityBuilder* builder, const EntityData& entityData) const;

private:
	Ref< db::Database > m_database;
};
	
	}
}

#endif	// traktor_world_ExternalEntityFactory_H
