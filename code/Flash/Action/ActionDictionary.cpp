/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Flash/Action/ActionDictionary.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionDictionary", ActionDictionary, Object)

ActionDictionary::ActionDictionary()
{
}

Ref< ActionDictionary > ActionDictionary::clone() const
{
	return new ActionDictionary(m_table);
}

void ActionDictionary::add(const ActionValue& value)
{
	m_table.push_back(value);
}

ActionDictionary::ActionDictionary(const AlignedVector< ActionValue >& table)
:	m_table(table)
{
}

	}
}
