/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_input_InputDeviceTouchGamepad_H
#define traktor_input_InputDeviceTouchGamepad_H

#import "Input/iOS/UITouchView.h"

#include <map>
#include "Input/IInputDevice.h"

namespace traktor
{
	namespace input
	{
	
class InputDeviceTouchGamepad
:	public IInputDevice
,	public ITouchViewCallback
{
	T_RTTI_CLASS;
	
public:
	InputDeviceTouchGamepad();
	
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
	struct IControl
	{
		virtual void begin(UITouch* touch) = 0;
		
		virtual void end(UITouch* touch) = 0;
		
		virtual void move(InputDeviceTouchGamepad* device, UITouch* touch) = 0;
	};
	
	struct Pad : public IControl
	{
		float axisX;
		float axisY;
		CGPoint origin;
				
		Pad()
		:	axisX(0.0f)
		,	axisY(0.0f)
		{
		}

		virtual void begin(UITouch* touch);
		
		virtual void end(UITouch* touch);
		
		virtual void move(InputDeviceTouchGamepad* device, UITouch* touch);
	};
	
	struct Button : public IControl
	{
		float value;
		
		Button()
		:	value(0.0f)
		{
		}

		virtual void begin(UITouch* touch);
		
		virtual void end(UITouch* touch);
		
		virtual void move(InputDeviceTouchGamepad* device, UITouch* touch);
	};
	
	bool m_landscape;
	CGPoint m_pivots[3];
	Pad m_leftPad;
	Pad m_rightPad;
	Button m_leftButton;
	Button m_rightButton;
	IControl* m_controls[4];
	std::map< UITouch*, IControl* > m_track;
};
	
	}
}

#endif	// traktor_input_InputDeviceTouchGamepad_H
