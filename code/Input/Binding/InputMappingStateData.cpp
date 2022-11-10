/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Input/Binding/InputMappingStateData.h"
#include "Input/Binding/InputStateData.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.input.InputMappingStateData", 0, InputMappingStateData, ISerializable)

void InputMappingStateData::setStateData(const std::wstring& id, InputStateData* data)
{
	if (data)
		m_stateData[id] = data;
	else
		m_stateData.remove(id);
}

const SmallMap< std::wstring, Ref< InputStateData > >& InputMappingStateData::getStateData() const
{
	return m_stateData;
}

void InputMappingStateData::serialize(ISerializer& s)
{
	s >> MemberSmallMap<
		std::wstring,
		Ref< InputStateData >,
		Member< std::wstring >,
		MemberRef< InputStateData >
	>(L"stateData", m_stateData);
}

	}
}
