#ifndef traktor_input_DeviceControlManager_H
#define traktor_input_DeviceControlManager_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Input/InputTypes.h"

namespace traktor
{
	namespace input
	{

class DeviceControl;
class InputSystem;

/*! \brief Device control manager.
 * \ingroup Input
 */
class DeviceControlManager : public Object
{
	T_RTTI_CLASS;

public:
	DeviceControlManager(InputSystem* inputSystem);

	int32_t getDeviceControlCount(InputCategory category);

	Ref< DeviceControl > getDeviceControl(InputCategory category, InputDefaultControlType controlType, bool analogue, int32_t index);

	void update();

	InputSystem* getInputSystem() { return m_inputSystem; }

private:
	Ref< InputSystem > m_inputSystem;
	RefArray< DeviceControl > m_deviceControls;
};

	}
}

#endif	// traktor_input_DeviceControlManager_H
