#ifndef traktor_input_InputDeviceTouch_H
#define traktor_input_InputDeviceTouch_H

#import "Input/iPhone/UITouchView.h"

#include "Input/IInputDevice.h"

namespace traktor
{
	namespace input
	{
	
class InputDeviceTouch
:	public IInputDevice
,	public ITouchViewCallback
{
	T_RTTI_CLASS(InputDeviceTouch)
	
public:
	InputDeviceTouch();
	
	bool create(void* nativeWindowHandle);

	virtual std::wstring getName() const;

	virtual InputCategory getCategory() const;

	virtual bool isConnected() const;

	virtual int getControlCount();

	virtual std::wstring getControlName(int control);

	virtual bool isControlAnalogue(int control) const;

	virtual float getControlValue(int control);

	virtual bool getDefaultControl(InputDefaultControlType controlType, int& control) const;

	virtual void resetState();

	virtual void readState();

	virtual bool supportRumble() const;

	virtual void setRumble(const InputRumble& rumble);
	
	// ITouchViewCallback
	
	virtual void touchesBegan(NSSet* touches, UIEvent* event);

	virtual void touchesMoved(NSSet* touches, UIEvent* event);

	virtual void touchesEnded(NSSet* touches, UIEvent* event);

	virtual void touchesCancelled(NSSet* touches, UIEvent* event);

private:
	struct Pad
	{
		float axisX;
		float axisY;
		CGPoint origin;
		UITouch* touch;
		
		Pad()
		:	axisX(0.0f)
		,	axisY(0.0f)
		,	touch(0)
		{
		}
	};
	
	bool m_landscape;
	CGFloat m_center;	
	Pad m_leftPad;
	Pad m_rightPad;
};
	
	}
}

#endif	// traktor_input_InputDeviceTouch_H
