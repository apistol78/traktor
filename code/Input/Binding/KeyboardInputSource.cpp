#include "Core/Io/StringOutputStream.h"
#include "Input/IInputDevice.h"
#include "Input/InputSystem.h"
#include "Input/Binding/DeviceControl.h"
#include "Input/Binding/DeviceControlManager.h"
#include "Input/Binding/KeyboardInputSource.h"
#include "Input/Binding/ValueDigital.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.KeyboardInputSource", KeyboardInputSource, Object)

KeyboardInputSource::KeyboardInputSource(
	const std::vector< InputDefaultControlType >& controlTypes,
	DeviceControlManager* deviceControlManager
)
{
	for (std::vector< InputDefaultControlType >::const_iterator i = controlTypes.begin(); i != controlTypes.end(); ++i)
	{
		m_deviceControls.push_back(deviceControlManager->getDeviceControl(
			CtKeyboard,
			*i,
			false,
			0
		));
	}
}

std::wstring KeyboardInputSource::getDescription() const
{
	StringOutputStream ss;
	for (RefArray< DeviceControl >::const_iterator i = m_deviceControls.begin(); i != m_deviceControls.end(); ++i)
	{
		if (i != m_deviceControls.begin())
			ss << L" + ";

		std::wstring controlName = i->getControlName();
		ss << controlName;
	}
	return ss.str();
}

float KeyboardInputSource::read(float T, float dT)
{
	if (m_deviceControls.empty())
		return 0.0f;

	for (RefArray< DeviceControl >::const_iterator i = m_deviceControls.begin(); i != m_deviceControls.end(); ++i)
	{
		if (!asBoolean((*i)->getCurrentValue()))
			return 0.0f;
	}

	return 1.0f;
}

	}
}
