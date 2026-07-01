/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/BoxedObjectInspector.h"

#include "Core/Class/Boxed.h"

namespace traktor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.BoxedObjectInspector", 0, BoxedObjectInspector, IObjectInspector)

bool BoxedObjectInspector::supportType(const TypeInfo& objectType) const
{
	return is_type_of< Boxed >(objectType);
}

std::wstring BoxedObjectInspector::toString(const ITypedObject* object) const
{
	return mandatory_non_null_type_cast< const Boxed* >(object)->toString();
}

}
