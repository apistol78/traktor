/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeDirectory.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.AttributeDirectory", AttributeDirectory, Attribute)

Ref< Attribute > AttributeDirectory::internalClone() const
{
	return new AttributeDirectory();
}

}
