#ifndef traktor_input_KeyboardDeviceAndroid_H
#define traktor_input_KeyboardDeviceAndroid_H

#include "Core/Platform.h"
#include "Core/Containers/CircularVector.h"
#include "Input/IInputDevice.h"

struct AInputEvent;

namespace traktor
{
	namespace input
	{

class KeyboardDeviceAndroid : public IInputDevice
{
	T_RTTI_CLASS;

public:
	KeyboardDeviceAndroid();

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

	static void showSoftKeyboard();

	static void hideSoftKeyboard();

private:
	friend class InputDriverAndroid;

	static struct ANativeActivity* ms_activity;
	CircularVector< KeyEvent, 128 > m_keyEvents;

	void handleInput(AInputEvent* event);
};

	}
}

#endif	// traktor_input_KeyboardDeviceAndroid_H
