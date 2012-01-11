#ifndef traktor_input_MouseDeviceWin32_H
#define traktor_input_MouseDeviceWin32_H

#include "Input/IInputDevice.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_WIN32_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{

class T_DLLCLASS MouseDeviceWin32 : public IInputDevice
{
	T_RTTI_CLASS;

public:
	MouseDeviceWin32();

	virtual std::wstring getName() const;

	virtual InputCategory getCategory() const;

	virtual bool isConnected() const;

	virtual int32_t getControlCount();

	virtual std::wstring getControlName(int32_t control);

	virtual bool isControlAnalogue(int32_t control) const;

	virtual bool isControlStable(int32_t control) const;

	virtual float getControlValue(int32_t control);

	virtual bool getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const;

	virtual void resetState();

	virtual void readState();

	virtual bool supportRumble() const;

	virtual void setRumble(const InputRumble& rumble);

private:
	friend class InputDriverWin32;

	bool m_connected;
	HWND m_hWndActive;
	POINT m_cursorPosition;
	bool m_haveCursorPosition;
	float m_axisX;
	float m_axisY;
	float m_positionX;
	float m_positionY;
	float m_button1;
	float m_button2;
	float m_button3;
};

	}
}

#endif	// traktor_input_MouseDeviceWin32_H
