#pragma once

#import "Input/iOS/UITouchView.h"

#include "Input/IInputDevice.h"

namespace traktor
{
	namespace input
	{

class MouseDeviceiOS
:	public IInputDevice
,	public ITouchViewCallback
{
	T_RTTI_CLASS;

public:
	MouseDeviceiOS();

	bool create(void* nativeWindowHandle);

	virtual std::wstring getName() const override final;

	virtual InputCategory getCategory() const override final;

	virtual bool isConnected() const override final;

	virtual int32_t getControlCount() override final;

	virtual std::wstring getControlName(int32_t control) override final;

	virtual bool isControlAnalogue(int32_t control) const override final;

	virtual bool isControlStable(int32_t control) const override final;

	virtual float getControlValue(int32_t control) override final;

	virtual bool getControlRange(int32_t control, float& outMin, float& outMax) const override final;

	virtual bool getDefaultControl(InputDefaultControlType controlType, bool analogue, int32_t& control) const override final;

	virtual bool getKeyEvent(KeyEvent& outEvent) override final;

	virtual void resetState() override final;

	virtual void readState() override final;

	virtual bool supportRumble() const override final;

	virtual void setRumble(const InputRumble& rumble) override final;

	virtual void setExclusive(bool exclusive) override final;

	// ITouchViewCallback

	virtual void touchesBegan(NSSet* touches, UIEvent* event) override final;

	virtual void touchesMoved(NSSet* touches, UIEvent* event) override final;

	virtual void touchesEnded(NSSet* touches, UIEvent* event) override final;

	virtual void touchesCancelled(NSSet* touches, UIEvent* event) override final;

	// Misc

	void setLandscape(bool landscape) { m_landscape = landscape; }

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

