/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/Attribute.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Attribute", Attribute, Object)

Attribute::Attribute()
:	m_next(0)
{
}

const Attribute* Attribute::find(const TypeInfo& type) const
{
	for (const Attribute* i = this; i; i = i->m_next)
	{
		if (&type_of(i) == &type)
			return i;
	}
	return 0;
}

const Attribute& Attribute::operator | (const Attribute& rh)
{
	m_next = &rh;
	return *this;
}

}
