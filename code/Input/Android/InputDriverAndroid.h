/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_input_InputDriverAndroid_H
#define traktor_input_InputDriverAndroid_H

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

	virtual bool create(const SystemApplication& sysapp, const SystemWindow& syswin, uint32_t inputCategories) T_OVERRIDE T_FINAL;

	virtual int getDeviceCount() T_OVERRIDE T_FINAL;

	virtual Ref< IInputDevice > getDevice(int index) T_OVERRIDE T_FINAL;

	virtual UpdateResult update() T_OVERRIDE T_FINAL;

	virtual void notifyHandleInput(DelegateInstance* instance, AInputEvent* event) T_OVERRIDE T_FINAL;

	virtual void notifyHandleEvents(DelegateInstance* instance) T_OVERRIDE T_FINAL;

private:
	DelegateInstance* m_instance;
	ASensorEventQueue* m_sensorQueue;
	Ref< KeyboardDeviceAndroid > m_keyboardDevice;
	Ref< MouseDeviceAndroid > m_mouseDevice;
	Ref< TouchDeviceAndroid > m_touchDevice;
	Ref< SensorDeviceAndroid > m_accelerationDevice;
	Ref< SensorDeviceAndroid > m_orientationDevice;
	RefArray< IInputDevice > m_devices;
};

	}
}

#endif	// traktor_input_InputDriverAndroid_H
