#include "Input/ReplayInputDevice.h"
#include "Input/RecordInputScript.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.ReplayInputDevice", ReplayInputDevice, InputDevice)

ReplayInputDevice::ReplayInputDevice(InputDevice* inputDevice, RecordInputScript* inputScript, bool loop)
:	m_inputDevice(inputDevice)
,	m_inputScript(inputScript)
,	m_loop(loop)
,	m_frame(0)
{
}

std::wstring ReplayInputDevice::getName() const
{
	return m_inputDevice->getName();
}

InputCategory ReplayInputDevice::getCategory() const
{
	return m_inputDevice->getCategory();
}

int ReplayInputDevice::getControlCount()
{
	return m_inputDevice->getControlCount();
}

std::wstring ReplayInputDevice::getControlName(int control)
{
	return m_inputDevice->getControlName(control);
}

bool ReplayInputDevice::isControlAnalogue(int control) const
{
	return m_inputDevice->isControlAnalogue(control);
}

float ReplayInputDevice::getControlValue(int control)
{
	return m_inputScript->getInputValue(m_frame, control);
}

bool ReplayInputDevice::getDefaultControl(InputDefaultControlType controlType, int& control) const
{
	return m_inputDevice->getDefaultControl(controlType, control);
}

void ReplayInputDevice::resetState()
{
	m_frame = 0;
}

void ReplayInputDevice::readState()
{
	m_frame++;
	if (m_loop && m_frame > m_inputScript->getLastFrame())
		m_frame = 0;
}

bool ReplayInputDevice::supportRumble() const
{
	return false;
}

void ReplayInputDevice::setRumble(const InputRumble& rumble)
{
}

	}
}
