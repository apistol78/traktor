/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Animation/Editor/LwsParser/LwsValue.h"
#include "Core/Misc/String.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.LwsValue", LwsValue, LwsNode)

LwsValue::LwsValue(const std::vector< std::wstring >& pieces)
:	LwsNode(pieces[0])
{
	if (pieces.size() > 1)
		m_values.insert(m_values.begin(), pieces.begin() + 1, pieces.end());
}

uint32_t LwsValue::getCount() const
{
	return uint32_t(m_values.size());
}

int32_t LwsValue::getInteger(uint32_t index) const
{
	T_ASSERT (index < m_values.size());
	return parseString< int32_t >(m_values[index]);
}

float LwsValue::getFloat(uint32_t index) const
{
	T_ASSERT (index < m_values.size());
	return parseString< float >(m_values[index]);
}

const std::wstring& LwsValue::getString(uint32_t index) const
{
	T_ASSERT (index < m_values.size());
	return m_values[index];
}

	}
}
