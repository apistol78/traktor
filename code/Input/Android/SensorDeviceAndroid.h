/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_input_SensorDeviceAndroid_H
#define traktor_input_SensorDeviceAndroid_H

//#include <android/looper.h>
#include <android/sensor.h>
//#include <hardware/sensors.h>

#include "Core/Platform.h"
#include "Input/IInputDevice.h"

struct AInputEvent;

namespace traktor
{
	namespace input
	{

class SensorDeviceAndroid : public IInputDevice
{
	T_RTTI_CLASS;

public:
	SensorDeviceAndroid(InputCategory category, ASensorRef sensor);

	virtual std::wstring getName() const T_OVERRIDE T_FINAL;

	virtual InputCategory getCategory() const T_OVERRIDE T_FINAL;

	virtual bool isConnected() const T_OVERRIDE T_FINAL;

	virtual int32_t getControlCount() T_OVERRIDE T_FINAL;

	virtual std::wstring getControlName(int32_t control) T_OVERRIDE T_FINAL;

	virtual bool isControlAnalogue(int32_t control) const T_OVERRIDE T_FINAL;

	virtual bool isControlStable(int32_t control) const T_OVERRIDE T_FINAL;

	virtual float getControlValue(int32_t control) T_OVERRIDE T_FINAL;

	virtual bool getControlRange(int32_t control, float& outMin, float& outMax) const T_OVERRIDE T_FINAL;

	virtual bool getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const T_OVERRIDE T_FINAL;

	virtual bool getKeyEvent(KeyEvent& outEvent) T_OVERRIDE T_FINAL;

	virtual void resetState() T_OVERRIDE T_FINAL;

	virtual void readState() T_OVERRIDE T_FINAL;

	virtual bool supportRumble() const T_OVERRIDE T_FINAL;

	virtual void setRumble(const InputRumble& rumble) T_OVERRIDE T_FINAL;

	virtual void setExclusive(bool exclusive) T_OVERRIDE T_FINAL;

private:
	friend class InputDriverAndroid;

	InputCategory m_category;
	ASensorRef m_sensor;
	float m_values[3];

	void handleInput(const ASensorEvent& data);
};

	}
}

#endif	// traktor_input_SensorDeviceAndroid_H
