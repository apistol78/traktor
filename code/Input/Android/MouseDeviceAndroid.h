#ifndef traktor_input_MouseDeviceAndroid_H
#define traktor_input_MouseDeviceAndroid_H

#include "Core/Platform.h"
#include "Input/IInputDevice.h"

struct AInputEvent;

namespace traktor
{
	namespace input
	{

class MouseDeviceAndroid : public IInputDevice
{
	T_RTTI_CLASS;

public:
	MouseDeviceAndroid(const SystemWindow& syswin);

	virtual std::wstring getName() const;

	virtual InputCategory getCategory() const;

	virtual bool isConnected() const;

	virtual int32_t getControlCount();

	virtual std::wstring getControlName(int32_t control);

	virtual bool isControlAnalogue(int32_t control) const;

	virtual bool isControlStable(int32_t control) const;

	virtual float getControlValue(int32_t control);

	virtual bool getControlRange(int32_t control, float& outMin, float& outMax) const;

	virtual bool getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const;

	virtual bool getKeyEvent(KeyEvent& outEvent);

	virtual void resetState();

	virtual void readState();

	virtual bool supportRumble() const;

	virtual void setRumble(const InputRumble& rumble);

	virtual void setExclusive(bool exclusive);

private:
	friend class InputDriverAndroid;

	SystemWindow m_syswim;
	float m_axisX;
	float m_axisY;
	float m_positionX;
	float m_positionY;
	bool m_button;

	void handleInput(AInputEvent* event);
};

	}
}

#endif	// traktor_input_MouseDeviceAndroid_H
