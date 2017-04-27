/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ToolFormCocoa_H
#define traktor_ui_ToolFormCocoa_H

#import <Cocoa/Cocoa.h>
#import "Ui/Cocoa/NSWindowDelegateProxy.h"

#include <map>
#include "Ui/Itf/IToolForm.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

class ToolFormCocoa
:	public IToolForm
,	public INSWindowEventsCallback
{
public:
	ToolFormCocoa(EventSubject* owner);
	
	// IToolForm implementation

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style) T_OVERRIDE;

	virtual void center() T_OVERRIDE;
	
	// IWidget implementation
	
	virtual void destroy() T_OVERRIDE;

	virtual void setParent(IWidget* parent) T_OVERRIDE;

	virtual void setText(const std::wstring& text) T_OVERRIDE;

	virtual std::wstring getText() const T_OVERRIDE;

	virtual void setToolTipText(const std::wstring& text) T_OVERRIDE;

	virtual void setForeground() T_OVERRIDE;

	virtual bool isForeground() const T_OVERRIDE;

	virtual void setVisible(bool visible) T_OVERRIDE;

	virtual bool isVisible(bool includingParents) const T_OVERRIDE;

	virtual void setActive() T_OVERRIDE;

	virtual void setEnable(bool enable) T_OVERRIDE;

	virtual bool isEnable() const T_OVERRIDE;

	virtual bool hasFocus() const T_OVERRIDE;

	virtual bool containFocus() const T_OVERRIDE;

	virtual void setFocus() T_OVERRIDE;

	virtual bool hasCapture() const T_OVERRIDE;

	virtual void setCapture() T_OVERRIDE;

	virtual void releaseCapture() T_OVERRIDE;

	virtual void startTimer(int interval, int id) T_OVERRIDE;
	
	virtual void stopTimer(int id) T_OVERRIDE;

	virtual void setOutline(const Point* p, int np) T_OVERRIDE;

	virtual void setRect(const Rect& rect) T_OVERRIDE;

	virtual Rect getRect() const T_OVERRIDE;

	virtual Rect getInnerRect() const T_OVERRIDE;

	virtual Rect getNormalRect() const T_OVERRIDE;

	virtual Size getTextExtent(const std::wstring& text) const T_OVERRIDE;

	virtual void setFont(const Font& font) T_OVERRIDE;

	virtual Font getFont() const T_OVERRIDE;

	virtual void setCursor(Cursor cursor) T_OVERRIDE;

	virtual Point getMousePosition(bool relative) const T_OVERRIDE;

	virtual Point screenToClient(const Point& pt) const T_OVERRIDE;

	virtual Point clientToScreen(const Point& pt) const T_OVERRIDE;

	virtual bool hitTest(const Point& pt) const T_OVERRIDE;

	virtual void setChildRects(const std::vector< IWidgetRect >& childRects) T_OVERRIDE;

	virtual Size getMinimumSize() const T_OVERRIDE;

	virtual Size getPreferedSize() const T_OVERRIDE;

	virtual Size getMaximumSize() const T_OVERRIDE;

	virtual void update(const Rect* rc, bool immediate) T_OVERRIDE;

	virtual void* getInternalHandle() T_OVERRIDE;

	virtual SystemWindow getSystemWindow() T_OVERRIDE;
	
	// INSWindowEventsCallback
	
	virtual void event_windowDidMove() T_OVERRIDE;
	
	virtual void event_windowDidResize() T_OVERRIDE;
	
	virtual bool event_windowShouldClose() T_OVERRIDE;

	virtual void event_windowDidBecomeKey() T_OVERRIDE;
	
	virtual void event_windowDidResignKey() T_OVERRIDE;
	
	virtual void event_windowDidBecomeMain() T_OVERRIDE;
	
	virtual void event_windowDidResignMain() T_OVERRIDE;

private:
	EventSubject* m_owner;
	NSWindow* m_window;
	std::map< int, NSTimer* > m_timers;
	
	void callbackTimer(void* controlId);
};

	}
}

#endif	// traktor_ui_ToolFormCocoa_H
