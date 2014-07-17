#ifndef traktor_ui_WidgetUniteImpl_H
#define traktor_ui_WidgetUniteImpl_H

#include "Ui/Itf/IEventSubject.h"
#include "Ui/Itf/IUserWidget.h"
#include "Ui/Itf/IWidgetFactory.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;
class IWidgetFactory;

template < typename ControlType >
class WidgetUniteImpl
:	public ControlType
,	public IEventSubject
{
public:
	WidgetUniteImpl(IWidgetFactory* nativeWidgetFactory, IEventSubject* owner)
	:	m_nativeWidgetFactory(nativeWidgetFactory)
	,	m_nativeWidget(0)
	,	m_owner(owner)
	{
	}

	virtual void destroy()
	{
		m_nativeWidget->destroy();
		delete this;
	}

	virtual void setParent(IWidget* parent)
	{
		IWidget* parentNativeWidget = static_cast< IWidget* >(parent->getInternalHandle());
		m_nativeWidget->setParent(parentNativeWidget);
	}

	virtual void setText(const std::wstring& text)
	{
		m_nativeWidget->setText(text);
	}

	virtual std::wstring getText() const
	{
		return m_nativeWidget->getText();
	}

	virtual void setToolTipText(const std::wstring& text)
	{
		m_nativeWidget->setToolTipText(text);
	}

	virtual void setForeground()
	{
		m_nativeWidget->setForeground();
	}

	virtual bool isForeground() const
	{
		return m_nativeWidget->isForeground();
	}

	virtual void setVisible(bool visible)
	{
		m_nativeWidget->setVisible(visible);
	}

	virtual bool isVisible(bool includingParents) const
	{
		return m_nativeWidget->isVisible(includingParents);
	}

	virtual void setActive()
	{
		m_nativeWidget->setActive();
	}

	virtual void setEnable(bool enable)
	{
		m_nativeWidget->setEnable(enable);
	}

	virtual bool isEnable() const
	{
		return m_nativeWidget->isEnable();
	}

	virtual bool hasFocus() const
	{
		return m_nativeWidget->hasFocus();
	}

	virtual bool containFocus() const
	{
		return m_nativeWidget->containFocus();
	}

	virtual void setFocus()
	{
		m_nativeWidget->setFocus();
	}

	virtual bool hasCapture() const
	{
		return m_nativeWidget->hasCapture();
	}

	virtual void setCapture()
	{
		m_nativeWidget->setCapture();
	}

	virtual void releaseCapture()
	{
		m_nativeWidget->releaseCapture();
	}

	virtual void startTimer(int interval, int id)
	{
		m_nativeWidget->startTimer(interval, id);
	}

	virtual void stopTimer(int id)
	{
		m_nativeWidget->stopTimer(id);
	}

	virtual void setOutline(const Point* p, int np)
	{
		m_nativeWidget->setOutline(p, np);
	}

	virtual void setRect(const Rect& rect)
	{
		m_nativeWidget->setRect(rect);
	}

	virtual Rect getRect() const
	{
		return m_nativeWidget->getRect();
	}

	virtual Rect getInnerRect() const
	{
		return m_nativeWidget->getInnerRect();
	}

	virtual Rect getNormalRect() const
	{
		return m_nativeWidget->getNormalRect();
	}

	virtual Size getTextExtent(const std::wstring& text) const
	{
		return m_nativeWidget->getTextExtent(text);
	}

	virtual void setFont(const Font& font)
	{
		m_nativeWidget->setFont(font);
	}

	virtual Font getFont() const
	{
		return m_nativeWidget->getFont();
	}

	virtual void setCursor(Cursor cursor)
	{
		m_nativeWidget->setCursor(cursor);
	}

	virtual Point getMousePosition(bool relative) const
	{
		return m_nativeWidget->getMousePosition(relative);
	}

	virtual Point screenToClient(const Point& pt) const
	{
		return m_nativeWidget->screenToClient(pt);
	}

	virtual Point clientToScreen(const Point& pt) const
	{
		return m_nativeWidget->clientToScreen(pt);
	}

	virtual bool hitTest(const Point& pt) const
	{
		return m_nativeWidget->hitTest(pt);
	}

	virtual void setChildRects(const std::vector< IWidgetRect >& childRects)
	{
		m_nativeWidget->setChildRects(childRects);
	}

	virtual Size getMinimumSize() const
	{
		return m_nativeWidget->getMinimumSize();
	}

	virtual Size getPreferedSize() const
	{
		return m_nativeWidget->getPreferedSize();
	}

	virtual Size getMaximumSize() const
	{
		return m_nativeWidget->getMaximumSize();
	}

	virtual void update(const Rect* rc, bool immediate)
	{
		m_nativeWidget->update(rc, immediate);
	}

	virtual void* getInternalHandle()
	{
		return m_nativeWidget;
	}

	virtual void* getSystemHandle()
	{
		return m_nativeWidget->getSystemHandle();
	}

	virtual void raiseEvent(int32_t eventId, Event* event)
	{
	}

protected:
	IWidgetFactory* m_nativeWidgetFactory;
	IWidget* m_nativeWidget;
	IEventSubject* m_owner;
};

	}
}

#endif	// traktor_ui_WidgetUniteImpl_H
