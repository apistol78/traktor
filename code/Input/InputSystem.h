#ifndef traktor_input_InputSystem_H
#define traktor_input_InputSystem_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Input/InputTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class InputDriver;
class InputDevice;

class T_DLLCLASS InputSystem : public Object
{
	T_RTTI_CLASS(InputSystem)

public:
	void addDriver(InputDriver* inputDriver);

	void removeDriver(InputDriver* inputDriver);

	void addDevice(InputDevice* inputDevice);

	void removeDevice(InputDevice* inputDevice);

	int getDeviceCount() const;

	InputDevice* getDevice(int index);

	int getDeviceCount(InputCategory category) const;

	InputDevice* getDevice(InputCategory category, int index);

	bool update(float deltaTime);

private:
	RefArray< InputDriver > m_drivers;
	RefArray< InputDevice > m_devices;

	void updateDevices();
};

	}
}

#endif	// traktor_input_InputSystem_H
