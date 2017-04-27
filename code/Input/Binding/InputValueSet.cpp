/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Input/Binding/InputValueSet.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.InputValueSet", InputValueSet, Object)

void InputValueSet::set(handle_t valueId, float value)
{
	m_valueMap[valueId] = value;
}

float InputValueSet::get(handle_t valueId) const
{
	SmallMap< handle_t, float >::const_iterator i = m_valueMap.find(valueId);
	return i != m_valueMap.end() ? i->second : 0.0f;
}

	}
}
