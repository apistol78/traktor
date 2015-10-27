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

	virtual bool create(void* nativeHandle, const SystemWindow& systemWindow, uint32_t inputCategories);

	virtual int getDeviceCount();

	virtual Ref< IInputDevice > getDevice(int index);

	virtual UpdateResult update();

	virtual void notifyHandleInput(struct android_app* app, AInputEvent* event);

private:
	DelegateInstance* m_instance;
	Ref< MouseDeviceAndroid > m_mouseDevice;
	Ref< TouchDeviceAndroid > m_touchDevice;
	RefArray< IInputDevice > m_devices;
};

	}
}

#endif	// traktor_input_InputDriverAndroid_H
