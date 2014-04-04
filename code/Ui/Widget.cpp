#include "Ui/Widget.h"
#include "Ui/Application.h"
#include "Ui/Events/ChildEvent.h"
#include "Ui/Itf/IUserWidget.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Widget", Widget, EventSubject)

Widget::Widget()
:	m_widget(0)
,	m_parent(0)
{
}

Widget::~Widget()
{
	T_ASSERT (!m_widget);
}

bool Widget::create(Widget* parent, int style)
{
	if (!m_widget)
	{
		IUserWidget* widget = Application::getInstance()->getWidgetFactory()->createUserWidget(this);
		if (!widget)
		{
			log::error << L"Failed to create native widget peer (UserWidget)" << Endl;
			return false;
		}

		if (!widget->create(parent->getIWidget(), style))
		{
			widget->destroy();
			return false;
		}

		m_widget = widget;
	}

	link(parent);

	return true;
}

void Widget::destroy()
{
	// Ensure this instance is alive as unlink might otherwise
	// cause this instance to be collected as we will become "unreachable".
	T_ANONYMOUS_VAR(Ref< Widget >)(this);

	removeAllData();
	removeAllEventHandlers();

	while (m_firstChild)
		m_firstChild->destroy();

	T_ASSERT (!m_firstChild);
	T_ASSERT (!m_lastChild);

	unlink();

	if (m_widget)
	{
		m_widget->destroy();
		m_widget = 0;
	}
}

void Widget::setText(const std::wstring& text)
{
	T_ASSERT (m_widget);
	m_widget->setText(text);
}

std::wstring Widget::getText() const
{
	T_ASSERT (m_widget);
	return m_widget->getText();
}

void Widget::setToolTipText(const std::wstring& text)
{
	T_ASSERT (m_widget);
	return m_widget->setToolTipText(text);
}

void Widget::setForeground()
{
	T_ASSERT (m_widget);
	m_widget->setForeground();
}

bool Widget::isForeground() const
{
	T_ASSERT (m_widget);
	return m_widget->isForeground();
}

void Widget::setVisible(bool visible)
{
	T_ASSERT (m_widget);
	m_widget->setVisible(visible);
}

bool Widget::isVisible(bool includingParents) const
{
	T_ASSERT (m_widget);
	return m_widget->isVisible(includingParents);
}

void Widget::setEnable(bool enable)
{
	T_ASSERT (m_widget);
	m_widget->setEnable(enable);
}

bool Widget::isEnable() const
{
	T_ASSERT (m_widget);
	return m_widget->isEnable();
}

bool Widget::hasFocus() const
{
	T_ASSERT (m_widget);
	return m_widget->hasFocus();
}

bool Widget::containFocus() const
{
	T_ASSERT (m_widget);
	return m_widget->containFocus();
}

void Widget::setFocus()
{
	T_ASSERT (m_widget);
	m_widget->setFocus();
}

void Widget::setRect(const Rect& rect)
{
	T_ASSERT (m_widget);
	m_widget->setRect(rect);
	update();
}

Rect Widget::getRect() const
{
	T_ASSERT (m_widget);
	return m_widget->getRect();
}

Rect Widget::getInnerRect() const
{
	T_ASSERT (m_widget);
	return m_widget->getInnerRect();
}

Rect Widget::getNormalRect() const
{
	T_ASSERT (m_widget);
	return m_widget->getNormalRect();
}

Size Widget::getTextExtent(const std::wstring& text) const
{
	T_ASSERT (m_widget);
	return m_widget->getTextExtent(text);
}

void Widget::setFont(const Font& font)
{
	T_ASSERT (m_widget);
	m_widget->setFont(font);
}

Font Widget::getFont()
{
	T_ASSERT (m_widget);
	return m_widget->getFont();
}

void Widget::setCursor(Cursor cursor)
{
	T_ASSERT (m_widget);
	m_widget->setCursor(cursor);
}

void Widget::resetCursor()
{
	T_ASSERT (m_widget);
	m_widget->setCursor(CrArrow);
}

void Widget::update(const Rect* rc, bool immediate)
{
	T_ASSERT (m_widget);
	m_widget->update(rc, immediate);
}

void Widget::show()
{
	T_ASSERT (m_widget);
	m_widget->setVisible(true);
	m_widget->setActive();
}

void Widget::hide()
{
	T_ASSERT (m_widget);
	m_widget->setVisible(false);
}

void Widget::raise()
{
	//T_ASSERT (m_widget);
	//m_window->Raise();
}

void Widget::lower()
{
	//T_ASSERT (m_widget);
	//m_window->Lower();
}

bool Widget::hasCapture() const
{
	T_ASSERT (m_widget);
	return m_widget->hasCapture();
}

void Widget::setCapture()
{
	T_ASSERT (m_widget);
	m_widget->setCapture();
}

void Widget::releaseCapture()
{
	T_ASSERT (m_widget);
	m_widget->releaseCapture();
}

void Widget::startTimer(int interval, int id)
{
	T_ASSERT (m_widget);
	m_widget->startTimer(interval, id);
}

void Widget::stopTimer(int id)
{
	T_ASSERT (m_widget);
	m_widget->stopTimer(id);
}

void Widget::setOutline(const Point* p, int np)
{
	T_ASSERT (m_widget);
	m_widget->setOutline(p, np);
}

Point Widget::getMousePosition(bool relative) const
{
	T_ASSERT (m_widget);
	return m_widget->getMousePosition(relative);
}

Point Widget::screenToClient(const Point& pt) const
{
	T_ASSERT (m_widget);
	return m_widget->screenToClient(pt);
}

Point Widget::clientToScreen(const Point& pt) const
{
	T_ASSERT (m_widget);
	return m_widget->clientToScreen(pt);
}

bool Widget::hitTest(const Point& pt) const
{
	T_ASSERT (m_widget);
	return m_widget->hitTest(pt);
}

void Widget::setChildRects(const std::vector< WidgetRect >& childRects)
{
	T_ASSERT (m_widget);
	if (!childRects.empty())
	{
		std::vector< IWidgetRect > internalChildRects(childRects.size());
		for (uint32_t i = 0; i < childRects.size(); ++i)
		{
			internalChildRects[i].widget = childRects[i].widget->getIWidget();
			internalChildRects[i].rect = childRects[i].rect;
		}
		m_widget->setChildRects(internalChildRects);
	}
}

Size Widget::getMinimumSize() const
{
	T_ASSERT (m_widget);
	return m_widget->getMinimumSize();
}

Size Widget::getPreferedSize() const
{
	T_ASSERT (m_widget);
	return m_widget->getPreferedSize();
}

Size Widget::getMaximumSize() const
{
	T_ASSERT (m_widget);
	return m_widget->getMaximumSize();
}

bool Widget::acceptLayout() const
{
	return true;
}

void Widget::link(Widget* parent)
{
	if (parent)
	{
		m_parent = parent;
		m_previousSibling = parent->m_lastChild;
		m_nextSibling = 0;
		if (parent->m_lastChild != 0)
			parent->m_lastChild->m_nextSibling = this;
		else
			parent->m_firstChild = this;
		parent->m_lastChild = this;

		// Invoke event on parent.
		ChildEvent childEvent(m_parent, this, true);
		m_parent->raiseEvent(EiChild, &childEvent);
	}
}

void Widget::unlink()
{
	if (m_parent)
	{
		if (m_parent->m_firstChild == this)
			m_parent->m_firstChild = m_nextSibling;
		if (m_parent->m_lastChild == this)
			m_parent->m_lastChild = m_previousSibling;
	}
	
	if (m_nextSibling)
		m_nextSibling->m_previousSibling = m_previousSibling;
	if (m_previousSibling)
		m_previousSibling->m_nextSibling = m_nextSibling;

	// Invoke event on parent.
	if (m_parent)
	{
		ChildEvent childEvent(m_parent, this, false);
		m_parent->raiseEvent(EiChild, &childEvent);
	}

	m_parent = 0;
	m_nextSibling = 0;
	m_previousSibling = 0;
}

void Widget::setParent(Widget* parent)
{
	unlink();
	if (parent)
	{
		m_widget->setParent(parent->getIWidget());
		link(parent);
	}
	else
		m_widget->setParent(0);
}

Widget* Widget::getParent() const
{
	return m_parent;
}

Ref< Widget > Widget::getPreviousSibling() const
{
	return m_previousSibling;
}

Ref< Widget > Widget::getNextSibling() const
{
	return m_nextSibling;
}

Ref< Widget > Widget::getFirstChild() const
{
	return m_firstChild;
}

Ref< Widget > Widget::getLastChild() const
{
	return m_lastChild;
}

void Widget::addChildEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiChild, eventHandler);
}

void Widget::addKeyEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiKey, eventHandler);
}

void Widget::addKeyDownEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiKeyDown, eventHandler);
}

void Widget::addKeyUpEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiKeyUp, eventHandler);
}

void Widget::addMoveEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiMove, eventHandler);
}

void Widget::addSizeEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiSize, eventHandler);
}

void Widget::addShowEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiShow, eventHandler);
}

void Widget::addButtonDownEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiButtonDown, eventHandler);
}

void Widget::addButtonUpEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiButtonUp, eventHandler);
}

void Widget::addDoubleClickEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiDoubleClick, eventHandler);
}

void Widget::addMouseMoveEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiMouseMove, eventHandler);
}

void Widget::addMouseWheelEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiMouseWheel, eventHandler);
}

void Widget::addFocusEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiFocus, eventHandler);
}

void Widget::addPaintEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiPaint, eventHandler);
}

void Widget::addTimerEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiTimer, eventHandler);
}

void Widget::addFileDropEventHandler(EventHandler* eventHandler)
{
	addEventHandler(EiFileDrop, eventHandler);
}

IWidget* Widget::getIWidget() const
{
	return m_widget;
}

	}
}
