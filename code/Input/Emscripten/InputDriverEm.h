#ifndef traktor_input_InputDriverEm_H
#define traktor_input_InputDriverEm_H

#include "Core/RefArray.h"
#include "Input/IInputDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EMSCRIPTEN_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class MouseDeviceEm;

class T_DLLCLASS InputDriverEm : public IInputDriver
{
	T_RTTI_CLASS;

public:
	InputDriverEm();

	virtual ~InputDriverEm();

	virtual bool create(const SystemApplication& sysapp, const SystemWindow& syswin, uint32_t inputCategories) T_OVERRIDE T_FINAL;

	virtual int getDeviceCount() T_OVERRIDE T_FINAL;

	virtual Ref< IInputDevice > getDevice(int index) T_OVERRIDE T_FINAL;

	virtual UpdateResult update() T_OVERRIDE T_FINAL;

private:
	Ref< MouseDeviceEm > m_mouseDevice;
	RefArray< IInputDevice > m_devices;
};

	}
}

#endif	// traktor_input_InputDriverEm_H
