#include <android/looper.h>
#include <android_native_app_glue.h>
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Input/Android/InputDriverAndroid.h"
#include "Input/Android/KeyboardDeviceAndroid.h"
#include "Input/Android/MouseDeviceAndroid.h"
#include "Input/Android/SensorDeviceAndroid.h"
#include "Input/Android/TouchDeviceAndroid.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputDriverAndroid", 0, InputDriverAndroid, IInputDriver)

InputDriverAndroid::InputDriverAndroid()
:	m_instance(0)
,	m_sensorQueue(0)
{
}

InputDriverAndroid::~InputDriverAndroid()
{
	if (m_instance)
	{
		m_instance->removeDelegate(this);
		m_instance = 0;
	}
}

bool InputDriverAndroid::create(const SystemApplication& sysapp, const SystemWindow& syswin, uint32_t inputCategories)
{
	m_instance = sysapp.instance;
	if (!m_instance)
	{
		log::error << L"Unable to create Android input driver; Invalid instance handle." << Endl;
		return false;
	}

	m_instance->addDelegate(this);

	if (inputCategories & CtKeyboard)
	{
		m_keyboardDevice = new KeyboardDeviceAndroid();
		m_keyboardDevice->ms_activity = m_instance->getActivity();
		m_devices.push_back(m_keyboardDevice);
	}
	if (inputCategories & CtMouse)
	{
		m_mouseDevice = new MouseDeviceAndroid(syswin);
		m_devices.push_back(m_mouseDevice);
	}
	if (inputCategories & CtTouch)
	{
		m_touchDevice = new TouchDeviceAndroid(syswin);
		m_devices.push_back(m_touchDevice);
	}

	if (inputCategories & (CtAcceleration | CtOrientation))
	{
		ASensorManager* sensorManager = ASensorManager_getInstance();

		ASensorList sensorList = nullptr;
		int32_t sensorCount = ASensorManager_getSensorList(sensorManager, &sensorList);
		log::info << L"Found " << sensorCount << L" supported sensors." << Endl;
		
		m_sensorQueue = ASensorManager_createEventQueue(
			sensorManager,
			m_instance->getApplication()->looper,
			LOOPER_ID_USER,
			NULL,
			NULL
		);
		if (!m_sensorQueue)
		{
			log::error << L"Failed to create sensor event queue." << Endl;
			return false;
		}

		const int kTimeoutMicroSecs = (1000L/60)*1000;
		for (int32_t i = 0; i < sensorCount; i++)
		{
			ASensorRef sensor = sensorList[i];

			switch (ASensor_getType(sensor))
			{
			case ASENSOR_TYPE_ACCELEROMETER:
				if ((inputCategories & CtAcceleration) != 0 && !m_accelerationDevice)
				{
					if (
						ASensorEventQueue_enableSensor(m_sensorQueue, sensor) >= 0 &&
						ASensorEventQueue_setEventRate(m_sensorQueue, sensor, kTimeoutMicroSecs) >= 0
					)
					{
						log::info << L"Creating Android acceleration device \"" << mbstows(ASensor_getName(sensor)) << L"\"" << Endl;
						m_accelerationDevice = new SensorDeviceAndroid(CtAcceleration, sensor);
						m_devices.push_back(m_accelerationDevice);
					}
				}
				break;

			case ASENSOR_TYPE_GYROSCOPE:
				if ((inputCategories & CtOrientation) != 0 && !m_orientationDevice)
				{
					if (
						ASensorEventQueue_enableSensor(m_sensorQueue, sensor) >= 0 &&
						ASensorEventQueue_setEventRate(m_sensorQueue, sensor, kTimeoutMicroSecs) >= 0
					)
					{
						log::info << L"Creating Android orientation device \"" << mbstows(ASensor_getName(sensor)) << L"\"" << Endl;
						m_orientationDevice = new SensorDeviceAndroid(CtOrientation, sensor);
						m_devices.push_back(m_orientationDevice);
					}
				}
				break;
			}
		}
	}

	log::info << L"Android input driver created successfully." << Endl;
	return true;
}

int InputDriverAndroid::getDeviceCount()
{
	return int(m_devices.size());
}

Ref< IInputDevice > InputDriverAndroid::getDevice(int index)
{
	if (index < int(m_devices.size()))
		return m_devices[index];
	else
		return 0;
}

IInputDriver::UpdateResult InputDriverAndroid::update()
{
	return UrOk;
}

void InputDriverAndroid::notifyHandleInput(DelegateInstance* instance, AInputEvent* event)
{
	if (m_mouseDevice)
		m_mouseDevice->handleInput(event);
	if (m_touchDevice)
		m_touchDevice->handleInput(event);
	if (m_keyboardDevice)
		m_keyboardDevice->handleInput(event);
}

void InputDriverAndroid::notifyHandleEvents(DelegateInstance* instance)
{
	if (m_sensorQueue)
	{
		ASensorEvent data;
		while (ASensorEventQueue_getEvents(m_sensorQueue, &data, 1) > 0)
		{
			if (m_accelerationDevice)
				m_accelerationDevice->handleInput(data);
			if (m_orientationDevice)
				m_orientationDevice->handleInput(data);
		}
	}
}

	}
}
