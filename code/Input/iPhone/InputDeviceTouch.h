#ifndef traktor_input_InputDeviceTouch_H
#define traktor_input_InputDeviceTouch_H

#import "Input/iPhone/UITouchView.h"

#include <map>
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
	struct IControl
	{
		virtual void begin(UITouch* touch) = 0;
		
		virtual void end(UITouch* touch) = 0;
		
		virtual void move(InputDeviceTouch* device, UITouch* touch) = 0;
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
		
		virtual void move(InputDeviceTouch* device, UITouch* touch);
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
		
		virtual void move(InputDeviceTouch* device, UITouch* touch);
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

#endif	// traktor_input_InputDeviceTouch_H
