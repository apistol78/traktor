/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Rtti/ITypedObject.h"

namespace traktor
{

const TypeInfo ITypedObject::ms_typeInfo(
	L"traktor.ITypedObject",
	sizeof(ITypedObject),
	0,
	false,
	nullptr,
	nullptr
);

const TypeInfo& ITypedObject::getClassTypeInfo()
{
	return ms_typeInfo;
}

}
