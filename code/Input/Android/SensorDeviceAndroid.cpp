#include <android/input.h>
#include <android/native_window.h>
#include "Core/Log/Log.h"
#include "Core/Math/MathUtils.h"
#include "Input/Android/SensorDeviceAndroid.h"

namespace traktor
{
	namespace input
	{
		namespace
		{

const struct SensorControlMap
{
	const wchar_t* name;
	InputDefaultControlType controlType;
	bool analogue;
	bool stable;
	int32_t index;
}
c_sensorControlMap[] =
{
	{ L"X axis", DtAxisX, true, false, 0 },
	{ L"Y axis", DtAxisY, true, false, 1 },
	{ L"Z axis", DtAxisZ, true, false, 2 }
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.input.SensorDeviceAndroid", SensorDeviceAndroid, IInputDevice)

SensorDeviceAndroid::SensorDeviceAndroid(InputCategory category, ASensorRef sensor)
:	m_category(category)
,	m_sensor(sensor)
{
	resetState();
}

std::wstring SensorDeviceAndroid::getName() const
{
	return L"Sensor";
}

InputCategory SensorDeviceAndroid::getCategory() const
{
	return m_category;
}

bool SensorDeviceAndroid::isConnected() const
{
	return true;
}

int32_t SensorDeviceAndroid::getControlCount()
{
	return sizeof_array(c_sensorControlMap);
}

std::wstring SensorDeviceAndroid::getControlName(int32_t control)
{
	return c_sensorControlMap[control].name;
}

bool SensorDeviceAndroid::isControlAnalogue(int32_t control) const
{
	return c_sensorControlMap[control].analogue;
}

bool SensorDeviceAndroid::isControlStable(int32_t control) const
{
	return c_sensorControlMap[control].stable;
}

float SensorDeviceAndroid::getControlValue(int32_t control)
{
	return m_values[c_sensorControlMap[control].index];
}

bool SensorDeviceAndroid::getControlRange(int32_t control, float& outMin, float& outMax) const
{
	return false;
}

bool SensorDeviceAndroid::getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const
{
	for (uint32_t i = 0; i < sizeof_array(c_sensorControlMap); ++i)
	{
		const SensorControlMap& scm = c_sensorControlMap[i];
		if (scm.controlType == controlType && scm.analogue == analogue)
		{
			control = i;
			return true;
		}
	}
	return false;
}

bool SensorDeviceAndroid::getKeyEvent(KeyEvent& outEvent)
{
	return false;
}

void SensorDeviceAndroid::resetState()
{
	m_values[0] =
	m_values[1] =
	m_values[2] = 0.0f;
}

void SensorDeviceAndroid::readState()
{
}

bool SensorDeviceAndroid::supportRumble() const
{
	return false;
}

void SensorDeviceAndroid::setRumble(const InputRumble& /*rumble*/)
{
}

void SensorDeviceAndroid::setExclusive(bool exclusive)
{
}

void SensorDeviceAndroid::handleInput(const ASensorEvent& data)
{
	if (m_category == CtAcceleration && data.type == ASENSOR_TYPE_ACCELEROMETER)
	{
		m_values[0] = data.acceleration.x;
		m_values[1] = data.acceleration.y;
		m_values[2] = data.acceleration.z;
	}
	else if (m_category == CtOrientation && data.type == ASENSOR_TYPE_GYROSCOPE)
	{
		m_values[0] = data.magnetic.x;
		m_values[1] = data.magnetic.y;
		m_values[2] = data.magnetic.z;
	}
}

	}
}
