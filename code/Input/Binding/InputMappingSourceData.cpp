/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
#include "Input/Binding/IInputSourceData.h"
#include "Input/Binding/InputMappingSourceData.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.input.InputMappingSourceData", 0, InputMappingSourceData, ISerializable)

void InputMappingSourceData::setSourceData(const std::wstring& id, IInputSourceData* data)
{
	if (data != 0)
		m_sourceData[id] = data;
	else
		m_sourceData.erase(id);
}

IInputSourceData* InputMappingSourceData::getSourceData(const std::wstring& id)
{
	std::map< std::wstring, Ref< IInputSourceData > >::const_iterator i = m_sourceData.find(id);
	return i != m_sourceData.end() ? i->second : 0;
}

const std::map< std::wstring, Ref< IInputSourceData > >& InputMappingSourceData::getSourceData() const
{
	return m_sourceData;
}

void InputMappingSourceData::serialize(ISerializer& s)
{
	s >> MemberStlMap<
		std::wstring,
		Ref< IInputSourceData >,
		MemberStlPair<
			std::wstring,
			Ref< IInputSourceData >,
			Member< std::wstring >,
			MemberRef< IInputSourceData >
		>
	>(L"sourceData", m_sourceData);
}

	}
}
