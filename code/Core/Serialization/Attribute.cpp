/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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
