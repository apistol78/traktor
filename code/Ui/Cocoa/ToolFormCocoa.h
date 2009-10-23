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

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style);

	virtual void center();
	
	// IWidget implementation
	
	virtual void destroy();

	virtual void setParent(IWidget* parent);

	virtual void setText(const std::wstring& text);

	virtual std::wstring getText() const;

	virtual void setToolTipText(const std::wstring& text);

	virtual void setForeground();

	virtual bool isForeground() const;

	virtual void setVisible(bool visible);

	virtual bool isVisible(bool includingParents) const;

	virtual void setActive();

	virtual void setEnable(bool enable);

	virtual bool isEnable() const;

	virtual bool hasFocus() const;

	virtual bool containFocus() const;

	virtual void setFocus();

	virtual bool hasCapture() const;

	virtual void setCapture();

	virtual void releaseCapture();

	virtual void startTimer(int interval, int id);
	
	virtual void stopTimer(int id);

	virtual void setOutline(const Point* p, int np);

	virtual void setRect(const Rect& rect);

	virtual Rect getRect() const;

	virtual Rect getInnerRect() const;

	virtual Rect getNormalRect() const;

	virtual Size getTextExtent(const std::wstring& text) const;

	virtual void setFont(const Font& font);

	virtual Font getFont() const;

	virtual void setCursor(Cursor cursor);

	virtual Point getMousePosition(bool relative) const;

	virtual Point screenToClient(const Point& pt) const;

	virtual Point clientToScreen(const Point& pt) const;

	virtual bool hitTest(const Point& pt) const;

	virtual void setChildRects(const std::vector< IWidgetRect >& childRects);

	virtual Size getMinimumSize() const;

	virtual Size getPreferedSize() const;

	virtual Size getMaximumSize() const;

	virtual void update(const Rect* rc, bool immediate);

	virtual void* getInternalHandle();

	virtual void* getSystemHandle();
	
	// INSWindowEventsCallback
	
	virtual void event_windowDidMove();
	
	virtual void event_windowDidResize();

private:
	EventSubject* m_owner;
	NSWindow* m_window;
	std::map< int, NSTimer* > m_timers;
	
	void callbackTimer(void* controlId);
};

	}
}

#endif	// traktor_ui_ToolFormCocoa_H
