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
