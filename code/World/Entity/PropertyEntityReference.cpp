/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/PropertyEntityReference.h"

#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "World/Entity.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.world.PropertyEntityReference", 0, PropertyEntityReference, IPropertyValue)

PropertyEntityReference::PropertyEntityReference(const Guid& entityId)
:	m_entityId(entityId)
{
}

PropertyEntityReference::value_type_t PropertyEntityReference::get(const IPropertyValue* value)
{
	const PropertyEntityReference* reference = dynamic_type_cast< const PropertyEntityReference* >(value);
	return reference ? reference->m_entityId : Guid();
}

Ref< IPropertyValue > PropertyEntityReference::join(const IPropertyValue* right) const
{
	return right->clone();
}

Ref< IPropertyValue > PropertyEntityReference::clone() const
{
	return new PropertyEntityReference(m_entityId);
}

void PropertyEntityReference::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"value", m_entityId, AttributeType(type_of< Entity >()));
}

}
