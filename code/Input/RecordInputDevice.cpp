#include "Input/RecordInputDevice.h"
#include "Input/RecordInputScript.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.RecordInputDevice", RecordInputDevice, IInputDevice)

RecordInputDevice::RecordInputDevice(IInputDevice* inputDevice, RecordInputScript* inputScript)
:	m_inputDevice(inputDevice)
,	m_inputScript(inputScript)
,	m_frame(0)
{
}

std::wstring RecordInputDevice::getName() const
{
	return m_inputDevice->getName();
}

InputCategory RecordInputDevice::getCategory() const
{
	return m_inputDevice->getCategory();
}

bool RecordInputDevice::isConnected() const
{
	return true;
}

int RecordInputDevice::getControlCount()
{
	return m_inputDevice->getControlCount();
}

std::wstring RecordInputDevice::getControlName(int control)
{
	return m_inputDevice->getControlName(control);
}

bool RecordInputDevice::isControlAnalogue(int control) const
{
	return m_inputDevice->isControlAnalogue(control);
}

float RecordInputDevice::getControlValue(int control)
{
	float value = m_inputDevice->getControlValue(control);
	m_inputScript->addInputValue(m_frame, control, value);
	return value;
}

bool RecordInputDevice::getDefaultControl(InputDefaultControlType controlType, int& control) const
{
	return m_inputDevice->getDefaultControl(controlType, control);
}

void RecordInputDevice::resetState()
{
	m_frame = 0;
}

void RecordInputDevice::readState()
{
	m_frame++;
}

bool RecordInputDevice::supportRumble() const
{
	return m_inputDevice->supportRumble();
}

void RecordInputDevice::setRumble(const InputRumble& rumble)
{
	m_inputDevice->setRumble(rumble);
}

	}
}
