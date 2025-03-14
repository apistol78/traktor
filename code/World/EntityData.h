/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Math/Transform.h"
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"
#include "World/WorldTypes.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

class IEntityComponentData;

/*! Entity data.
 * \ingroup World
 *
 * Serialized data used to create runtime entities.
 * The entity data is stored separately in order to
 * save runtime memory as the data structure is
 * normally tossed away after when the entity has
 * been created.
 */
class T_DLLCLASS EntityData : public ISerializable
{
	T_RTTI_CLASS;

public:
	/*! Set unique ID of entity data.
	 * \param id New unique ID.
	 */
	void setId(const Guid& id);

	/*! Get unique ID of entity data.
	 * \return Unique ID of entity data.
	 */
	const Guid& getId() const;

	/*! Set name of entity data.
	 * \param name New name of entity data.
	 */
	void setName(const std::wstring& name);

	/*! Get name of entity data.
	 * \return Name of entity data.
	 */
	const std::wstring& getName() const;

	/*! Set entity initial state.
	 * \param state New state.
	 * \param mask Mask which part of state should be set.
	 * \return Entity data result state.
	 */
	EntityState setState(const EntityState& state, const EntityState& mask);

	/*! Get entity initial state.
	 * \return Entity data current state.
	 */
	const EntityState& getState() const;

	/*! Set transform of entity data.
	 * \param transform New transform of entity data.
	 */
	virtual void setTransform(const Transform& transform);

	/*! Get transform of entity data.
	 * \return Current entity data transform.
	 */
	virtual const Transform& getTransform() const;

	/*! Set component.
	 *
	 * Only one component instance of each type is supported,
	 * thus existing instance will be replaced.
	 *
	 * \param component Component instance to set.
	 */
	void setComponent(IEntityComponentData* component);

	/*! Remove component.
	 *
	 * \param component Component instance to remove.
	 * \return True if component removed.
	 */
	bool removeComponent(const IEntityComponentData* component);

	/*! Get component of type.
	 *
	 * \param componentType Type of component.
	 * \return Component instance matching type.
	 */
	IEntityComponentData* getComponent(const TypeInfo& componentType) const;

	/*! Get component of type.
	 *
	 * \param ComponentDataType Type of component.
	 * \return Component instance matching type.
	 */
	template < typename ComponentDataType >
	ComponentDataType* getComponent() const
	{
		return checked_type_cast< ComponentDataType* >(getComponent(type_of< ComponentDataType >()));
	}

	/*! Set components. */
	void setComponents(const RefArray< IEntityComponentData >& components);

	/*! Get components.
	 *
	 * \return Array of all components.
	 */
	const RefArray< IEntityComponentData >& getComponents() const;

	virtual void serialize(ISerializer& s) override;

private:
	Guid m_id;
	std::wstring m_name;
	EntityState m_state;
	Transform m_transform;
	RefArray< IEntityComponentData > m_components;
};

}
