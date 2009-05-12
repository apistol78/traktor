#ifndef traktor_world_EntityManager_H
#define traktor_world_EntityManager_H

#include <map>
#include "Core/Object.h"
#include "Core/Heap/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class Entity;

/*! \brief Entity manager.
 * \ingroup World
 *
 * Entity manager is a class to help
 * accessing entities based on various
 * queries.
 */
class T_DLLCLASS EntityManager : public Object
{
	T_RTTI_CLASS(EntityManager)

public:
	/*! \brief Add entity to manager.
	 *
	 * \param entity Entity to add.
	 */
	void addEntity(Entity* entity);

	/*! \brief Insert entity in manager.
	 *
	 * This is method checks to ensure entity isn't
	 * added multiple times.
	 *
	 * \param entity Entity to insert.
	 */
	void insertEntity(Entity* entity);

	/*! \brief Remove entity from manager.
	 *
	 * \param entity Entity to remove.
	 */
	void removeEntity(Entity* entity);

	/*! \brief Get all entities of certain type.
	 *
	 * \param entityType Type of entity.
	 * \param outEntities Array of found entities.
	 * \return Number of entities found.
	 */
	uint32_t getEntitiesOf(const Type& entityType, RefArray< Entity >& outEntities) const;

	/*! \brief Get number of entities which are a certain type.
	 *
	 * \param entityType Type of entity.
	 * \return Number of entities of type.
	 */
	uint32_t getEntityCount(const Type& entityType) const;

	/*! \brief Get entity of certain type.
	 *
	 * \param entityType Type of entity.
	 * \param index Index of entity of same type.
	 * \return Found entity, null if no entity found.
	 */
	Entity* getEntity(const Type& entityType, uint32_t index) const;

	/*! \brief Get all entities.
	 *
	 * \return Array with all entities.
	 */
	const RefArray< Entity >& getEntities() const
	{
		return m_entities;
	}

	/*! \brief Get all entities of certain type.
	 *
	 * \param EntityType Type of entity.
	 * \param outEntities Array of found entities.
	 * \return Number of entities found.
	 */
	template < typename EntityType >
	int getEntitiesOf(RefArray< EntityType >& outEntities) const
	{
		return getEntitiesOf(EntityType::getClassType(), *reinterpret_cast< RefArray< Entity >* >(&outEntities));
	}

	/*! \brief Get number of entities of certain type.
	 *
	 * \param EntityType Type of entity.
	 * \return Number of entities of type.
	 */
	template < typename EntityType >
	uint32_t getEntityCount() const
	{
		return getEntityCount(EntityType::getClassType());
	}

	/*! \brief Get first entity of a certain type.
	 *
	 * \param EntityType Type of entity.
	 * \param index Index of entity of same type.
	 * \return Found entity, null if no entity found.
	 */
	template < typename EntityType >
	EntityType* getEntity(uint32_t index) const
	{
		return static_cast< EntityType* >(getEntity(EntityType::getClassType(), index));
	}

private:
	struct Range
	{
		uint32_t start;
		uint32_t count;
	};

	RefArray< Entity > m_entities;
	std::map< const Type*, Range > m_typeRanges;
};

	}
}

#endif	// traktor_world_EntityManager_H
