#ifndef traktor_input_InputDriverAndroid_H
#define traktor_input_InputDriverAndroid_H

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
class TouchDeviceAndroid;

class T_DLLCLASS InputDriverAndroid
:	public IInputDriver
,	public DelegateInstance::IDelegate
{
	T_RTTI_CLASS;

public:
	InputDriverAndroid();

	virtual ~InputDriverAndroid();

	virtual bool create(void* nativeHandle, const SystemWindow& systemWindow, uint32_t inputCategories) T_OVERRIDE T_FINAL;

	virtual int getDeviceCount() T_OVERRIDE T_FINAL;

	virtual Ref< IInputDevice > getDevice(int index) T_OVERRIDE T_FINAL;

	virtual UpdateResult update() T_OVERRIDE T_FINAL;

	virtual void notifyHandleInput(DelegateInstance* instance, AInputEvent* event) T_OVERRIDE T_FINAL;

private:
	DelegateInstance* m_instance;
	Ref< KeyboardDeviceAndroid > m_keyboardDevice;
	Ref< MouseDeviceAndroid > m_mouseDevice;
	Ref< TouchDeviceAndroid > m_touchDevice;
	RefArray< IInputDevice > m_devices;
};

	}
}

#endif	// traktor_input_InputDriverAndroid_H
