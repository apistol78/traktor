#ifndef traktor_input_InputDriverPs3_H
#define traktor_input_InputDriverPs3_H

#include "Core/RefArray.h"
#include "Input/IInputDriver.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_PS3_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class InputDevicePs3;

class T_DLLCLASS InputDriverPs3 : public IInputDriver
{
	T_RTTI_CLASS;

public:
	InputDriverPs3(int padCount = 4);

	virtual bool create(const SystemApplication& sysapp, const SystemWindow& syswin, uint32_t inputCategories);

	virtual int getDeviceCount();

	virtual Ref< IInputDevice > getDevice(int index);

	virtual UpdateResult update();

private:
	RefArray< InputDevicePs3 > m_devices;
};

	}
}

#endif	// traktor_input_InputDriverPs3_H
