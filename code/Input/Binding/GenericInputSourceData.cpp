/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Input/Binding/EnumKeys.h"
#include "Input/Binding/GenericInputSource.h"
#include "Input/Binding/GenericInputSourceData.h"

namespace traktor::input
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.GenericInputSourceData", 4, GenericInputSourceData, IInputSourceData)

GenericInputSourceData::GenericInputSourceData()
:	m_category(InputCategory::Unknown)
,	m_controlType(DefaultControl::Invalid)
,	m_analogue(false)
,	m_normalize(false)
,	m_index(-1)
{
}

GenericInputSourceData::GenericInputSourceData(
	InputCategory category,
	DefaultControl controlType,
	bool analogue,
	bool normalize
)
:	m_category(category)
,	m_controlType(controlType)
,	m_analogue(analogue)
,	m_normalize(normalize)
,	m_index(-1)
{
}

GenericInputSourceData::GenericInputSourceData(
	InputCategory category,
	int32_t index,
	DefaultControl controlType,
	bool analogue,
	bool normalize
)
:	m_category(category)
,	m_controlType(controlType)
,	m_analogue(analogue)
,	m_normalize(normalize)
,	m_index(index)
{
}

void GenericInputSourceData::setCategory(InputCategory category)
{
	m_category = category;
}

InputCategory GenericInputSourceData::getCategory() const
{
	return m_category;
}

void GenericInputSourceData::setControlType(DefaultControl controlType)
{
	m_controlType = controlType;
}

DefaultControl GenericInputSourceData::getControlType() const
{
	return m_controlType;
}

void GenericInputSourceData::setAnalogue(bool analogue)
{
	m_analogue = analogue;
}

bool GenericInputSourceData::isAnalogue() const
{
	return m_analogue;
}

void GenericInputSourceData::setIndex(int32_t index)
{
	m_index = index;
}

void GenericInputSourceData::setNormalize(bool normalize)
{
	m_normalize = normalize;
}

bool GenericInputSourceData::normalize() const
{
	return m_normalize;
}

int32_t GenericInputSourceData::getIndex() const
{
	return m_index;
}

Ref< IInputSource > GenericInputSourceData::createInstance(DeviceControlManager* deviceControlManager) const
{
	return new GenericInputSource(this, deviceControlManager);
}

void GenericInputSourceData::serialize(ISerializer& s)
{
	if (s.getVersion() >= 4)
	{
		s >> MemberEnum< InputCategory >(L"category", m_category, g_InputCategory_Keys);
		s >> MemberEnum< DefaultControl >(L"controlType", m_controlType, g_DefaultControl_Keys);
	}
	else
	{
		s >> MemberEnum< InputCategory >(L"category", m_category, g_InputCategory_Keys_Old);
		s >> MemberEnum< DefaultControl >(L"controlType", m_controlType, g_DefaultControl_Keys_Old);
	}

	s >> Member< bool >(L"analogue", m_analogue);

	if (s.getVersion() <= 2)
		s >> ObsoleteMember< bool >(L"inverted");

	if (s.getVersion() >= 2)
		s >> Member< bool >(L"normalize", m_normalize);

	s >> Member< int32_t >(L"index", m_index);
}

}
