/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Input/Xi/InputDriverXi.h"
#include "Input/Xi/InputDeviceXi.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputDriverXi", 0, InputDriverXi, IInputDriver)

InputDriverXi::InputDriverXi(DWORD deviceCount)
{
	for (DWORD i = 0; i < deviceCount; ++i)
		m_devices.push_back(new InputDeviceXi(i));
}

bool InputDriverXi::create(const SystemApplication& sysapp, const SystemWindow& syswin, uint32_t inputCategories)
{
	return true;
}

int InputDriverXi::getDeviceCount()
{
	return int(m_devices.size());
}

Ref< IInputDevice > InputDriverXi::getDevice(int index)
{
	return m_devices[index];
}

IInputDriver::UpdateResult InputDriverXi::update()
{
	return UrOk;
}

	}
}