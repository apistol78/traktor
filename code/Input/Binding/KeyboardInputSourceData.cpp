/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Core/Serialization/MemberStl.h"
#include "Input/Binding/EnumKeys.h"
#include "Input/Binding/KeyboardInputSource.h"
#include "Input/Binding/KeyboardInputSourceData.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

class MemberInputDefaultControlType : public MemberEnum< InputDefaultControlType >
{
public:
	MemberInputDefaultControlType(const wchar_t* const name, InputDefaultControlType& ref)
	:	MemberEnum< InputDefaultControlType >(name, ref, g_InputDefaultControlType_Keys)
	{
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.KeyboardInputSourceData", 0, KeyboardInputSourceData, IInputSourceData)

void KeyboardInputSourceData::addControlType(InputDefaultControlType controlType)
{
	m_controlTypes.push_back(controlType);
}

const std::vector< InputDefaultControlType >& KeyboardInputSourceData::getControlTypes() const
{
	return m_controlTypes;
}

Ref< IInputSource > KeyboardInputSourceData::createInstance(DeviceControlManager* deviceControlManager) const
{
	return new KeyboardInputSource(m_controlTypes, deviceControlManager);
}

void KeyboardInputSourceData::serialize(ISerializer& s)
{
	s >> MemberStlVector< InputDefaultControlType, MemberInputDefaultControlType >(L"controlTypes", m_controlTypes);
}
	
	}
}
