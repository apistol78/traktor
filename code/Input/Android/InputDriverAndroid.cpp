#include "Core/Log/Log.h"
#include "Input/Android/InputDriverAndroid.h"
#include "Input/Android/MouseDeviceAndroid.h"
#include "Input/Android/TouchDeviceAndroid.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputDriverAndroid", 0, InputDriverAndroid, IInputDriver)

InputDriverAndroid::InputDriverAndroid()
:	m_instance(0)
{
}

InputDriverAndroid::~InputDriverAndroid()
{
	if (m_instance)
	{
		m_instance->removeDelegate(this);
		m_instance = 0;
	}
}

bool InputDriverAndroid::create(void* nativeHandle, const SystemWindow& systemWindow, uint32_t inputCategories)
{
	m_instance = reinterpret_cast< DelegateInstance* >(nativeHandle);
	if (!m_instance)
	{
		log::error << L"Unable to create Android input driver; Invalid instance handle." << Endl;
		return false;
	}

	m_instance->addDelegate(this);

	if (inputCategories & CtMouse)
	{
		m_mouseDevice = new MouseDeviceAndroid();
		m_devices.push_back(m_mouseDevice);
	}
	if (inputCategories & CtTouch)
	{
		m_touchDevice = new TouchDeviceAndroid(systemWindow);
		m_devices.push_back(m_touchDevice);
	}

	log::info << L"Android input driver created successfully." << Endl;
	return true;
}

int InputDriverAndroid::getDeviceCount()
{
	return int(m_devices.size());
}

Ref< IInputDevice > InputDriverAndroid::getDevice(int index)
{
	if (index < int(m_devices.size()))
		return m_devices[index];
	else
		return 0;
}

IInputDriver::UpdateResult InputDriverAndroid::update()
{
	return UrOk;
}

void InputDriverAndroid::notifyHandleInput(struct android_app* app, AInputEvent* event)
{
	if (m_mouseDevice)
		m_mouseDevice->handleInput(event);
	if (m_touchDevice)
		m_touchDevice->handleInput(event);
}

	}
}
