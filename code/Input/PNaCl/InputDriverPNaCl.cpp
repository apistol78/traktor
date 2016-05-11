#include <ppapi/cpp/input_event.h>
#include "Core/Log/Log.h"
#include "Input/PNaCl/InputDriverPNaCl.h"
#include "Input/PNaCl/MouseDevicePNaCl.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputDriverPNaCl", 0, InputDriverPNaCl, IInputDriver)

InputDriverPNaCl::InputDriverPNaCl()
:	m_instance(0)
{
}

InputDriverPNaCl::~InputDriverPNaCl()
{
	if (m_instance)
	{
		m_instance->removeDelegate(this);
		m_instance = 0;
	}
}

bool InputDriverPNaCl::create(const SystemApplication& sysapp, const SystemWindow& syswin, uint32_t inputCategories)
{
	m_instance = sysapp.instance;
	if (!m_instance)
	{
		log::error << L"Unable to create PNaCl input driver; Invalid instance handle." << Endl;
		return false;
	}

	m_instance->addDelegate(this);
	m_instance->RequestInputEvents(PP_INPUTEVENT_CLASS_MOUSE /* | PP_INPUTEVENT_CLASS_KEYBOARD */);

	if (inputCategories & CtMouse)
	{
		m_mouseDevice = new MouseDevicePNaCl();
		m_devices.push_back(m_mouseDevice);
	}

	log::info << L"PNaCl input driver created successfully." << Endl;
	return true;
}

int InputDriverPNaCl::getDeviceCount()
{
	return int(m_devices.size());
}

Ref< IInputDevice > InputDriverPNaCl::getDevice(int index)
{
	if (index < int(m_devices.size()))
		return m_devices[index];
	else
		return 0;
}

IInputDriver::UpdateResult InputDriverPNaCl::update()
{
	return UrOk;
}

bool InputDriverPNaCl::notifyHandleInputEvent(const pp::InputEvent& event)
{
	switch (event.GetType())
	{
	case PP_INPUTEVENT_TYPE_MOUSEDOWN:
	case PP_INPUTEVENT_TYPE_MOUSEUP:
	case PP_INPUTEVENT_TYPE_MOUSEMOVE:
		m_mouseDevice->consumeEvent(event);
		return true;

	default:
		return false;
	}
}

	}
}
