#ifndef traktor_input_MouseDeviceX11_H
#define traktor_input_MouseDeviceX11_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput2.h>
#include "Input/IInputDevice.h"

namespace traktor
{
	namespace input
	{

class MouseDeviceX11 : public IInputDevice
{
	T_RTTI_CLASS;

public:
	MouseDeviceX11(Display* display, Window window, int deviceId);

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
	Display* m_display;
	Window m_window;
	int m_deviceId;
	bool m_connected;
	bool m_haveCursorPosition;
	float m_axisX;
	float m_axisY;
	float m_positionX;
	float m_positionY;
	float m_button1;
	float m_button2;
	float m_button3;
	int m_width;
	int m_height;
};

	}
}

#endif	// traktor_input_MouseDeviceX11_H
