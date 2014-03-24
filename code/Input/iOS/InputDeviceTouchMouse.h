#ifndef traktor_input_InputDeviceTouchMouse_H
#define traktor_input_InputDeviceTouchMouse_H

#import "Input/iOS/UITouchView.h"

#include "Input/IInputDevice.h"

namespace traktor
{
	namespace input
	{
	
class InputDeviceTouchMouse
:	public IInputDevice
,	public ITouchViewCallback
{
	T_RTTI_CLASS;
	
public:
	InputDeviceTouchMouse();
	
	bool create(void* nativeWindowHandle);

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
	
	// ITouchViewCallback
	
	virtual void touchesBegan(NSSet* touches, UIEvent* event);

	virtual void touchesMoved(NSSet* touches, UIEvent* event);

	virtual void touchesEnded(NSSet* touches, UIEvent* event);

	virtual void touchesCancelled(NSSet* touches, UIEvent* event);

private:
	bool m_landscape;
	float m_width;
	float m_height;
	float m_positionX;
	float m_positionY;
	bool m_button;
};
	
	}
}

#endif	// traktor_input_InputDeviceTouchMouse_H
