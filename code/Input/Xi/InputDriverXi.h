#ifndef traktor_input_InputDriverXi_H
#define traktor_input_InputDriverXi_H

#if defined(_XBOX)
#	include <xtl.h>
#else
#	include <windows.h>
#	include <xinput.h>
#endif
#include "Core/RefArray.h"
#include "Input/IInputDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_XI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class InputDeviceXi;

class T_DLLCLASS InputDriverXi : public IInputDriver
{
	T_RTTI_CLASS;

public:
	InputDriverXi(DWORD deviceCount = 4);

	virtual bool create(const SystemApplication& sysapp, const SystemWindow& syswin, uint32_t inputCategories) T_OVERRIDE T_FINAL;

	virtual int getDeviceCount() T_OVERRIDE T_FINAL;

	virtual Ref< IInputDevice > getDevice(int index) T_OVERRIDE T_FINAL;

	virtual UpdateResult update() T_OVERRIDE T_FINAL;

private:
	RefArray< InputDeviceXi > m_devices;
};

	}
}

#endif	// traktor_input_InputDriverXi_H
