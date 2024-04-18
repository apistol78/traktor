/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/ObjectStore.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ObjectStore", ObjectStore, Object)

void ObjectStore::set(Object* object)
{
	const TypeInfo& objectType = type_of(object);
	auto it = std::remove_if(m_objects.begin(), m_objects.end(), [&](Object* object) {
		return is_type_of(
			type_of(object),
			objectType
		);
	});
	m_objects.erase(it, m_objects.end());
	m_objects.push_back(object);
}

bool ObjectStore::unset(Object* object)
{
	return m_objects.remove(object);
}

Object* ObjectStore::get(const TypeInfo& objectType) const
{
	auto it = std::find_if(m_objects.begin(), m_objects.end(), [&](Object* object) {
		return is_type_of(
			objectType,
			type_of(object)
		);
	});
	return it != m_objects.end() ? *it : nullptr;
}

}