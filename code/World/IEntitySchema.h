/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_world_IEntitySchema_H
#define traktor_world_IEntitySchema_H

#include "Core/Object.h"
#include "Core/RefArray.h"

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

/*! \brief Entity schema interface.
 * \ingroup World
 *
 * Entity schema are blue-prints of entities
 * collected when entities are created and thus
 * are useful for resolving entity relationships
 * externally.
 */
class T_DLLCLASS IEntitySchema : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Insert entity into schema.
	 *
	 * \param parentEntity Parent entity.
	 * \param name Instance name.
	 * \param entity Entity to add.
	 */
	virtual void insertEntity(const Entity* parentEntity, const std::wstring& name, Entity* entity) = 0;

	/*! \brief
	 */
	virtual Entity* getEntity(uint32_t index) const = 0;

	/*! \brief Get named entity.
	 *
	 * \param name Name of entity.
	 * \param index Index of entity.
	 * \return Found entity, null if no entity found.
	 */
	virtual Entity* getEntity(const std::wstring& name, uint32_t index = 0) const = 0;

	/*! \brief Get entity of certain type.
	 *
	 * \param entityType Type of entity.
	 * \param index Index of entity.
	 * \return Found entity, null if no entity found.
	 */
	virtual Entity* getEntity(const TypeInfo& entityType, uint32_t index = 0) const = 0;

	/*! \brief Get named entity of certain type.
	 *
	 * \param entityType Type of entity.
	 * \param name Name of entity instance.
	 * \return Found entity, null if no entity found.
	 */
	virtual Entity* getEntity(const std::wstring& name, const TypeInfo& entityType, uint32_t index = 0) const = 0;

	/*! \brief Get child entity.
	 *
	 * \param index Index of entity.
	 * \return Found entity, null if no entity found.
	 */
	virtual Entity* getChildEntity(const Entity* parentEntity, uint32_t index = 0) const = 0;

	/*! \brief Get named child entity.
	 *
	 * \param name Name of entity.
	 * \param index Index of entity.
	 * \return Found entity, null if no entity found.
	 */
	virtual Entity* getChildEntity(const Entity* parentEntity, const std::wstring& name, uint32_t index = 0) const = 0;

	/*! \brief Get entity of certain type.
	 *
	 * \param entityType Type of entity.
	 * \param index Index of entity.
	 * \return Found entity, null if no entity found.
	 */
	virtual Entity* getChildEntity(const Entity* parentEntity, const TypeInfo& entityType, uint32_t index = 0) const = 0;

	/*! \brief Get named entity of certain type.
	 *
	 * \param entityType Type of entity.
	 * \param name Name of entity instance.
	 * \return Found entity, null if no entity found.
	 */
	virtual Entity* getChildEntity(const Entity* parentEntity, const std::wstring& name, const TypeInfo& entityType, uint32_t index = 0) const = 0;

	/*! \brief
	 */
	virtual uint32_t getEntities(RefArray< Entity >& outEntities) const = 0;

	/*! \brief Get named entities.
	 *
	 * \param name Name of entities.
	 * \param outEntities Array of found entities.
	 * \return Number of entities found.
	 */
	virtual uint32_t getEntities(const std::wstring& name, RefArray< Entity >& outEntities) const = 0;

	/*! \brief Get all entities of certain type.
	 *
	 * \param entityType Type of entity.
	 * \param outEntities Array of found entities.
	 * \return Number of entities found.
	 */
	virtual uint32_t getEntities(const TypeInfo& entityType, RefArray< Entity >& outEntities) const = 0;

	/*! \brief Get all named entities of certain type.
	 *
	 * \param entityType Type of entity.
	 * \param outEntities Array of found entities.
	 * \return Number of entities found.
	 */
	virtual uint32_t getEntities(const std::wstring& name, const TypeInfo& entityType, RefArray< Entity >& outEntities) const = 0;

	/*! \brief
	 */
	virtual uint32_t getChildEntities(const Entity* parentEntity, RefArray< Entity >& outEntities) const = 0;

	/*! \brief
	 */
	virtual uint32_t getChildEntities(const Entity* parentEntity, const std::wstring& name, RefArray< Entity >& outEntities) const = 0;

	/*! \brief
	 */
	virtual uint32_t getChildEntities(const Entity* parentEntity, const TypeInfo& entityType, RefArray< Entity >& outEntities) const = 0;

	/*! \brief
	 */
	virtual uint32_t getChildEntities(const Entity* parentEntity, const std::wstring& name, const TypeInfo& entityType, RefArray< Entity >& outEntities) const = 0;

	/*! \group Template type helpers.
	 *
	 * \{
	 */

	/*! \brief
	 */
	template < typename EntityType >
	EntityType* getEntity(uint32_t index = 0) const
	{
		Entity* entity = getEntity(type_of< EntityType >(), index);
		return static_cast< EntityType* >(entity);
	}

	/*! \brief
	 */
	template < typename EntityType >
	EntityType* getEntity(const std::wstring& name, uint32_t index = 0) const
	{
		Entity* entity = getEntity(name, type_of< EntityType >(), index);
		return static_cast< EntityType* >(entity);
	}

	/*! \brief
	 */
	template < typename EntityType >
	EntityType* getChildEntity(const Entity* parentEntity, uint32_t index = 0) const
	{
		Entity* entity = getChildEntity(parentEntity, type_of< EntityType >(), index);
		return static_cast< EntityType* >(entity);
	}

	/*! \brief
	 */
	template < typename EntityType >
	EntityType* getChildEntity(const Entity* parentEntity, const std::wstring& name, uint32_t index = 0) const
	{
		Entity* entity = getChildEntity(parentEntity, name, type_of< EntityType >(), index);
		return static_cast< EntityType* >(entity);
	}

	/*! \brief
	 */
	template < typename EntityType >
	int getEntities(RefArray< EntityType >& outEntities) const
	{
		return getEntities(
			type_of< EntityType >(),
			*reinterpret_cast< RefArray< Entity >* >(&outEntities)
		);
	}

	/*! \brief
	 */
	template < typename EntityType >
	int getEntities(const std::wstring& name, RefArray< EntityType >& outEntities) const
	{
		return getEntities(
			name,
			type_of< EntityType >(),
			*reinterpret_cast< RefArray< Entity >* >(&outEntities)
		);
	}

	/*! \brief
	 */
	template < typename EntityType >
	int getChildEntities(const Entity* parentEntity, RefArray< EntityType >& outEntities) const
	{
		return getChildEntities(
			parentEntity,
			type_of< EntityType >(),
			*reinterpret_cast< RefArray< Entity >* >(&outEntities)
		);
	}

	/*! \brief
	 */
	template < typename EntityType >
	int getChildEntities(const Entity* parentEntity, const std::wstring& name, RefArray< EntityType >& outEntities) const
	{
		return getChildEntities(
			parentEntity,
			name,
			type_of< EntityType >(),
			*reinterpret_cast< RefArray< Entity >* >(&outEntities)
		);
	}

	/*! \} */
};

	}
}

#endif	// traktor_world_IEntitySchema_H
