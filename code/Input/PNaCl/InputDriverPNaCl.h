#ifndef traktor_input_InputDriverPNaCl_H
#define traktor_input_InputDriverPNaCl_H

#include "Core/RefArray.h"
#include "Core/System/PNaCl/DelegateInstance.h"
#include "Input/IInputDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_PNACL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class MouseDevicePNaCl;

class T_DLLCLASS InputDriverPNaCl
:	public IInputDriver
,	public DelegateInstance::IDelegate
{
	T_RTTI_CLASS;

public:
	InputDriverPNaCl();

	virtual ~InputDriverPNaCl();

	virtual bool create(const SystemApplication& sysapp, const SystemWindow& syswin, uint32_t inputCategories);

	virtual int getDeviceCount();

	virtual Ref< IInputDevice > getDevice(int index);

	virtual UpdateResult update();

	virtual bool notifyHandleInputEvent(const pp::InputEvent& event);

private:
	DelegateInstance* m_instance;
	Ref< MouseDevicePNaCl > m_mouseDevice;
	RefArray< IInputDevice > m_devices;
};

	}
}

#endif	// traktor_input_InputDriverPNaCl_H
