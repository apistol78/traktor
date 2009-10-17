#ifndef traktor_ui_WidgetCocoaImpl_H
#define traktor_ui_WidgetCocoaImpl_H

#import <Cocoa/Cocoa.h>

#include <map>
#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/Cocoa/NSTargetProxy.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Events/SizeEvent.h"
#include "Ui/EventSubject.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace ui
	{
	
class EventSubject;

template < typename ControlType, typename NSControlType >	
class WidgetCocoaImpl : public ControlType
{
public:
	typedef WidgetCocoaImpl< ControlType, NSControlType > class_t;

	WidgetCocoaImpl(EventSubject* owner)
	:	m_owner(owner)
	,	m_control(0)
	,	m_tracking(false)
	{
	}
	
	virtual void destroy()
	{
		if (m_control)
		{
			[m_control autorelease];
			m_control = 0;
		}
	}

	virtual void setParent(IWidget* parent)
	{
	}

	virtual void setText(const std::wstring& text)
	{
		[m_control setStringValue: makeNSString(text)];
	}

	virtual std::wstring getText() const
	{
		return fromNSString([m_control stringValue]);
	}

	virtual void setToolTipText(const std::wstring& text)
	{
	}

	virtual void setForeground()
	{
	}

	virtual bool isForeground() const
	{
		return false;
	}

	virtual void setVisible(bool visible)
	{
		[m_control setHidden: visible ? NO : YES];
	}

	virtual bool isVisible(bool includingParents) const
	{
		if (!includingParents)
			return [m_control isHidden] == NO;
		else
			return [m_control isHiddenOrHasHiddenAncestor] == NO;
	}

	virtual void setActive()
	{
	}

	virtual void setEnable(bool enable)
	{
		[m_control setEnabled: enable ? YES : NO];
	}

	virtual bool isEnable() const
	{
		return [m_control isEnabled] == YES;
	}

	virtual bool hasFocus() const
	{
		NSWindow* window = [m_control window];
		if (!window)
			return false;
			
		return [window firstResponder] == m_control;
	}

	virtual bool containFocus() const
	{
		return hasFocus();
	}

	virtual void setFocus()
	{
		NSWindow* window = [m_control window];
		if (window)
			[window makeFirstResponder: m_control];
	}

	virtual bool hasCapture() const
	{
		return m_tracking;
	}

	virtual void setCapture()
	{
		T_ASSERT (!m_tracking);
		
		NSCell* cell = [m_control cell];
		if (cell)
		{
			NSRect bounds = [m_control bounds];
			[cell trackMouse: nil inRect: bounds ofView: m_control untilMouseUp: NO];
			m_tracking = true;
		}
	}

	virtual void releaseCapture()
	{
		NSCell* cell = [m_control cell];
		if (cell && m_tracking)
		{
			NSPoint pt = NSMakePoint(0, 0);
			[cell stopTracking: pt at: pt inView: m_control mouseIsUp: NO];
			m_tracking = false;
		}
	}

	virtual void startTimer(int interval, int id)
	{
		ITargetProxyCallback* targetCallback = new TargetProxyCallbackImpl< class_t >(
			this,
			&class_t::callbackTimer,
			0
		);
	
		NSTargetProxy* targetProxy = [[[NSTargetProxy alloc] init] autorelease];
		[targetProxy setCallback: targetCallback];
			
		NSTimer* timer = [
			NSTimer scheduledTimerWithTimeInterval: (double)interval / 1000.0
			target: targetProxy
			selector: @selector(dispatchActionCallback:)
			userInfo: nil
			repeats: YES
		];
		
		m_timers[id] = timer;
	}
	
	virtual void stopTimer(int id)
	{
		std::map< int, NSTimer* >::iterator i = m_timers.find(id);
		if (i != m_timers.end())
		{
			[i->second release];
			m_timers.erase(i);
		}
	}

	virtual void setOutline(const Point* p, int np)
	{
	}

	virtual void setRect(const Rect& rect)
	{
		[m_control setFrame: makeNSRect(rect)];
		raiseSizeEvent();
	}

	virtual Rect getRect() const
	{
		NSRect rc = [m_control frame];
		return fromNSRect(rc);
	}

	virtual Rect getInnerRect() const
	{
		NSRect rc = [m_control frame];
		rc.origin.x =
		rc.origin.y = 0;
		return fromNSRect(rc);
	}

	virtual Rect getNormalRect() const
	{
		return Rect(0, 0, 0, 0);
	}

	virtual Size getTextExtent(const std::wstring& text) const
	{
		NSString* str = makeNSString(text);
		NSSize sz = [str sizeWithAttributes: NULL];
		return fromNSSize(sz);
	}

	virtual void setFont(const Font& font)
	{
	}

	virtual Font getFont() const
	{
		return Font();
	}

	virtual void setCursor(Cursor cursor)
	{
	}

	virtual Point getMousePosition(bool relative) const
	{
		return Point(0, 0);
	}

	virtual Point screenToClient(const Point& pt) const
	{
		return Point(0, 0);
	}

	virtual Point clientToScreen(const Point& pt) const
	{
		return Point(0, 0);
	}

	virtual bool hitTest(const Point& pt) const
	{
		return false;
	}

	virtual void setChildRects(const std::vector< IWidgetRect >& childRects)
	{
		for (std::vector< IWidgetRect >::const_iterator i = childRects.begin(); i != childRects.end(); ++i)
		{
			if (i->widget)
				i->widget->setRect(i->rect);
		}
	}

	virtual Size getMinimumSize() const
	{
		return Size(0, 0);
	}

	virtual Size getPreferedSize() const
	{
		NSSize idealSize = [[m_control cell] cellSize];
		return Size(idealSize.width, idealSize.height);
	}

	virtual Size getMaximumSize() const
	{
		return Size(65535, 65535);
	}

	virtual void update(const Rect* rc, bool immediate)
	{
		[m_control display];
	}

	virtual void* getInternalHandle()
	{
		return m_control;
	}

	virtual void* getSystemHandle()
	{
		return m_control;
	}

protected:
	EventSubject* m_owner;
	NSControlType* m_control;
	std::map< int, NSTimer* > m_timers;
	bool m_tracking;
	
	void raiseSizeEvent()
	{
		Size sz = getRect().getSize();
		SizeEvent s(m_owner, 0, sz);
		m_owner->raiseEvent(EiSize, &s);
	}
	
	void callbackTimer(void* controlId)
	{
		CommandEvent commandEvent(m_owner, 0);
		m_owner->raiseEvent(EiTimer, &commandEvent);
	}
};
	
	}
}

#endif	// traktor_ui_WidgetCocoaImpl_H
