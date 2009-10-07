#ifndef traktor_ui_WidgetCocoaImpl_H
#define traktor_ui_WidgetCocoaImpl_H

#import <Cocoa/Cocoa.h>

#include "Ui/Cocoa/UtilitiesCocoa.h"
#include "Ui/Itf/IWidget.h"
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
	WidgetCocoaImpl(EventSubject* owner)
	:	m_owner(owner)
	,	m_control(0)
	{
	}
	
	virtual void destroy()
	{
		[m_control release];
	}

	virtual void setParent(IWidget* parent)
	{
	}

	virtual void setText(const std::wstring& text)
	{
	}

	virtual std::wstring getText() const
	{
		return L"";
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
	}

	virtual bool isVisible(bool includingParents) const
	{
		return true;
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
		return false;
	}

	virtual bool containFocus() const
	{
		return false;
	}

	virtual void setFocus()
	{
	}

	virtual bool hasCapture() const
	{
		return false;
	}

	virtual void setCapture()
	{
	}

	virtual void releaseCapture()
	{
	}

	virtual void startTimer(int interval, int id)
	{
	}

	virtual void stopTimer(int id)
	{
	}

	virtual void setOutline(const Point* p, int np)
	{
	}

	virtual void setRect(const Rect& rect)
	{
		log::info << L"setRect " << rect.left << L", " << rect.top << L", " << rect.right << L", " << rect.bottom << Endl;
		[m_control setFrame: makeNSRect(m_control, rect)];
	}

	virtual Rect getRect() const
	{
		NSRect rc = [m_control frame];
		return fromNSRect(m_control, rc);
	}

	virtual Rect getInnerRect() const
	{
		NSRect rc = [m_control frame];
		NSRect inner = NSMakeRect(0, 0, rc.size.width, rc.size.height);
		return fromNSRect(m_control, rc);
	}

	virtual Rect getNormalRect() const
	{
		return Rect(0, 0, 0, 0);
	}

	virtual Size getTextExtent(const std::wstring& text) const
	{
		return Size(text.length() * 16, 16);
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
		return Size(0, 0);
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
};
	
	}
}

#endif	// traktor_ui_WidgetCocoaImpl_H
