/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Core/Settings/IPropertyValue.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

/*! Property value referring to an entity of the same scene, by the entity's guid.
 * \ingroup World
 *
 * For property groups, such as a script component's properties, that
 * point at another entity: the guid is the referred entity's own, so the entity can be
 * named and moved freely.
 */
class T_DLLCLASS PropertyEntityReference : public IPropertyValue
{
	T_RTTI_CLASS;

public:
	typedef Guid value_type_t;

	PropertyEntityReference() = default;

	explicit PropertyEntityReference(const Guid& entityId);

	static value_type_t get(const IPropertyValue* value);

	const Guid& getEntityId() const { return m_entityId; }

	virtual Ref< IPropertyValue > join(const IPropertyValue* right) const override final;

	virtual Ref< IPropertyValue > clone() const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	Guid m_entityId;
};

}
