#ifndef traktor_input_DeviceControl_H
#define traktor_input_DeviceControl_H

#include "Core/IRefCount.h"
#include "Core/Ref.h"
#include "Input/InputTypes.h"

namespace traktor
{
	namespace input
	{

class IInputDevice;

class DeviceControl : public RefCountImpl< IRefCount >
{
public:
	std::wstring getControlName() const;

	float getPreviousValue() const;

	float getCurrentValue() const;
	
private:
	friend class DeviceControlManager;

	DeviceControl();

	InputCategory m_category;
	InputDefaultControlType m_controlType;
	bool m_analogue;
	int32_t m_index;
	Ref< IInputDevice > m_device;
	int32_t m_control;
	float m_previousValue;
	float m_currentValue;
};

	}
}

#endif	// traktor_input_DeviceControl_H
