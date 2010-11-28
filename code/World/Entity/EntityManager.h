#ifndef traktor_world_EntityManager_H
#define traktor_world_EntityManager_H

#include <map>
#include "World/Entity/IEntityManager.h"

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

/*! \brief Entity manager.
 * \ingroup World
 *
 * Entity manager is a class to help
 * accessing entities based on various
 * queries.
 */
class T_DLLCLASS EntityManager : public IEntityManager
{
	T_RTTI_CLASS;

public:
	virtual void addEntity(const std::wstring& name, Entity* entity);

	virtual void insertEntity(const std::wstring& name, Entity* entity);

	virtual void removeEntity(Entity* entity);

	virtual Entity* getEntity(const std::wstring& name) const;

	virtual uint32_t getEntitiesOf(const TypeInfo& entityType, RefArray< Entity >& outEntities) const;

	virtual uint32_t getEntityCount(const TypeInfo& entityType) const;

	virtual Entity* getEntity(const TypeInfo& entityType, uint32_t index) const;

	virtual Entity* getEntity(const TypeInfo& entityType, const std::wstring& name) const;

private:
	typedef std::pair< std::wstring, Ref< Entity > > named_entity_t;
	typedef std::vector< named_entity_t > named_entity_vector_t;

	struct Range
	{
		uint32_t start;
		uint32_t count;
	};

	named_entity_vector_t m_entities;
	std::map< const TypeInfo*, Range > m_typeRanges;
};

	}
}

#endif	// traktor_world_EntityManager_H
