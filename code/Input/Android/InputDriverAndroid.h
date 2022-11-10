/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <android/sensor.h>
#include "Core/RefArray.h"
#include "Core/System/Android/DelegateInstance.h"
#include "Input/IInputDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_ANDROID_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class GamepadDeviceAndroid;
class KeyboardDeviceAndroid;
class MouseDeviceAndroid;
class SensorDeviceAndroid;
class TouchDeviceAndroid;

class T_DLLCLASS InputDriverAndroid
:	public IInputDriver
,	public DelegateInstance::IDelegate
{
	T_RTTI_CLASS;

public:
	InputDriverAndroid();

	virtual ~InputDriverAndroid();

	virtual bool create(const SystemApplication& sysapp, const SystemWindow& syswin, uint32_t inputCategories) override final;

	virtual int getDeviceCount() override final;

	virtual Ref< IInputDevice > getDevice(int index) override final;

	virtual UpdateResult update() override final;

	virtual void notifyHandleInput(DelegateInstance* instance, AInputEvent* event) override final;

	virtual void notifyHandleEvents(DelegateInstance* instance) override final;

private:
	DelegateInstance* m_instance;
	ASensorEventQueue* m_sensorQueue;
	Ref< KeyboardDeviceAndroid > m_keyboardDevice;
	Ref< MouseDeviceAndroid > m_mouseDevice;
	Ref< TouchDeviceAndroid > m_touchDevice;
	Ref< GamepadDeviceAndroid > m_gamepadDevice;
	Ref< SensorDeviceAndroid > m_accelerationDevice;
	Ref< SensorDeviceAndroid > m_orientationDevice;
	RefArray< IInputDevice > m_devices;
};

	}
}

