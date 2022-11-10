/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class Entity;

/*! Entity schema interface.
 * \ingroup World
 *
 * Entity schema are blue-prints of entities
 * collected when entities are created and thus
 * are useful for resolving entity relationships
 * externally.
 */
class T_DLLCLASS EntitySchema : public Object
{
	T_RTTI_CLASS;

public:
	/*! Insert entity into schema.
	 *
	 * \param parentEntity Parent entity.
	 * \param name Instance name.
	 * \param entity Entity to add.
	 */
	void insertEntity(const Entity* parentEntity, const std::wstring& name, Entity* entity);

	/*! \brief
	 */
	Entity* getEntity(uint32_t index) const;

	/*! Get named entity.
	 *
	 * \param name Name of entity.
	 * \param index Index of entity.
	 * \return Found entity, null if no entity found.
	 */
	Entity* getEntity(const std::wstring& name, uint32_t index) const;

	/*! Get entity of certain type.
	 *
	 * \param entityType Type of entity.
	 * \param index Index of entity.
	 * \return Found entity, null if no entity found.
	 */
	Entity* getEntity(const TypeInfo& entityType, uint32_t index) const;

	/*! Get named entity of certain type.
	 *
	 * \param entityType Type of entity.
	 * \param name Name of entity instance.
	 * \return Found entity, null if no entity found.
	 */
	Entity* getEntity(const std::wstring& name, const TypeInfo& entityType, uint32_t index) const;

	/*! Get child entity.
	 *
	 * \param index Index of entity.
	 * \return Found entity, null if no entity found.
	 */
	Entity* getChildEntity(const Entity* parentEntity, uint32_t index) const;

	/*! Get named child entity.
	 *
	 * \param name Name of entity.
	 * \param index Index of entity.
	 * \return Found entity, null if no entity found.
	 */
	Entity* getChildEntity(const Entity* parentEntity, const std::wstring& name, uint32_t index) const;

	/*! Get entity of certain type.
	 *
	 * \param entityType Type of entity.
	 * \param index Index of entity.
	 * \return Found entity, null if no entity found.
	 */
	Entity* getChildEntity(const Entity* parentEntity, const TypeInfo& entityType, uint32_t index) const;

	/*! Get named entity of certain type.
	 *
	 * \param entityType Type of entity.
	 * \param name Name of entity instance.
	 * \return Found entity, null if no entity found.
	 */
	Entity* getChildEntity(const Entity* parentEntity, const std::wstring& name, const TypeInfo& entityType, uint32_t index) const;

	/*! \brief
	 */
	uint32_t getEntities(RefArray< Entity >& outEntities) const;

	/*! Get named entities.
	 *
	 * \param name Name of entities.
	 * \param outEntities Array of found entities.
	 * \return Number of entities found.
	 */
	uint32_t getEntities(const std::wstring& name, RefArray< Entity >& outEntities) const;

	/*! Get all entities of certain type.
	 *
	 * \param entityType Type of entity.
	 * \param outEntities Array of found entities.
	 * \return Number of entities found.
	 */
	uint32_t getEntities(const TypeInfo& entityType, RefArray< Entity >& outEntities) const;

	/*! Get all named entities of certain type.
	 *
	 * \param entityType Type of entity.
	 * \param outEntities Array of found entities.
	 * \return Number of entities found.
	 */
	uint32_t getEntities(const std::wstring& name, const TypeInfo& entityType, RefArray< Entity >& outEntities) const;

	/*! \brief
	 */
	uint32_t getChildEntities(const Entity* parentEntity, RefArray< Entity >& outEntities) const;

	/*! \brief
	 */
	uint32_t getChildEntities(const Entity* parentEntity, const std::wstring& name, RefArray< Entity >& outEntities) const;

	/*! \brief
	 */
	uint32_t getChildEntities(const Entity* parentEntity, const TypeInfo& entityType, RefArray< Entity >& outEntities) const;

	/*! \brief
	 */
	uint32_t getChildEntities(const Entity* parentEntity, const std::wstring& name, const TypeInfo& entityType, RefArray< Entity >& outEntities) const;

	/*! \group Template type helpers.
	 *
	 * \{
	 */

	/*! \brief
	 */
	template < typename EntityType >
	EntityType* getEntity(uint32_t index) const
	{
		Entity* entity = getEntity(type_of< EntityType >(), index);
		return static_cast< EntityType* >(entity);
	}

	/*! \brief
	 */
	template < typename EntityType >
	EntityType* getEntity(const std::wstring& name, uint32_t index) const
	{
		Entity* entity = getEntity(name, type_of< EntityType >(), index);
		return static_cast< EntityType* >(entity);
	}

	/*! \brief
	 */
	template < typename EntityType >
	EntityType* getChildEntity(const Entity* parentEntity, uint32_t index) const
	{
		Entity* entity = getChildEntity(parentEntity, type_of< EntityType >(), index);
		return static_cast< EntityType* >(entity);
	}

	/*! \brief
	 */
	template < typename EntityType >
	EntityType* getChildEntity(const Entity* parentEntity, const std::wstring& name, uint32_t index) const
	{
		Entity* entity = getChildEntity(parentEntity, name, type_of< EntityType >(), index);
		return static_cast< EntityType* >(entity);
	}

	/*! \brief
	 */
	template < typename EntityType >
	int32_t getEntities(RefArray< EntityType >& outEntities) const
	{
		return getEntities(
			type_of< EntityType >(),
			*reinterpret_cast< RefArray< Entity >* >(&outEntities)
		);
	}

	/*! \brief
	 */
	template < typename EntityType >
	int32_t getEntities(const std::wstring& name, RefArray< EntityType >& outEntities) const
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
	int32_t getChildEntities(const Entity* parentEntity, RefArray< EntityType >& outEntities) const
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
	int32_t getChildEntities(const Entity* parentEntity, const std::wstring& name, RefArray< EntityType >& outEntities) const
	{
		return getChildEntities(
			parentEntity,
			name,
			type_of< EntityType >(),
			*reinterpret_cast< RefArray< Entity >* >(&outEntities)
		);
	}

	/*! \} */

private:
	struct EntityInfo
	{
		const Entity* parent;
		Entity* entity;
	};

	struct Indices
	{
		AlignedVector< uint32_t > children;
		SmallMap< std::wstring, AlignedVector< uint32_t > > name;
		SmallMap< const TypeInfo*, AlignedVector< uint32_t > > type;
	};

	AlignedVector< EntityInfo > m_entityInfo;
	Indices m_global;
	SmallMap< const Entity*, Indices > m_children;
};

}
