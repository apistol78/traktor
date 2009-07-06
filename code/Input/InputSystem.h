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

class IInputDriver;
class IInputDevice;

/*! \brief Input system.
 * \ingroup Input
 */
class T_DLLCLASS InputSystem : public Object
{
	T_RTTI_CLASS(InputSystem)

public:
	void addDriver(IInputDriver* inputDriver);

	void removeDriver(IInputDriver* inputDriver);

	void addDevice(IInputDevice* inputDevice);

	void removeDevice(IInputDevice* inputDevice);

	int getDeviceCount() const;

	IInputDevice* getDevice(int index);

	int getDeviceCount(InputCategory category) const;

	IInputDevice* getDevice(InputCategory category, int index, bool connected);

	bool update(float deltaTime);

private:
	RefArray< IInputDriver > m_drivers;
	RefArray< IInputDevice > m_devices;

	void updateDevices();
};

	}
}

#endif	// traktor_input_InputSystem_H
