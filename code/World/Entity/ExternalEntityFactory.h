#ifndef traktor_world_ExternalEntityFactory_H
#define traktor_world_ExternalEntityFactory_H

#include "Core/Heap/Ref.h"
#include "World/Entity/IEntityFactory.h"

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
class T_DLLCLASS ExternalEntityFactory : public IEntityFactory
{
	T_RTTI_CLASS(ExternalEntityFactory)
	
public:
	ExternalEntityFactory(db::Database* database);

	virtual const TypeSet getEntityTypes() const;

	virtual Ref< Entity > createEntity(IEntityBuilder* builder, const std::wstring& name, const EntityData& entityData, const Object* instanceData) const;

private:
	Ref< db::Database > m_database;
};
	
	}
}

#endif	// traktor_world_ExternalEntityFactory_H
