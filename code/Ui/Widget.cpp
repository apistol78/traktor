/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Containers/StaticVector.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Ui/Application.h"
#include "Ui/StyleSheet.h"
#include "Ui/Widget.h"
#include "Ui/Itf/IUserWidget.h"
#include "Ui/Virtual/VirtualWidget.h"

namespace traktor::ui
{
namespace
{

// Embedded native widgets own a window inside a virtual tree, i.e. system
// window hosts such as 3d views; top-level windows and virtual widgets are not.
bool isNativeEmbedded(IWidget* peer)
{
	return peer != nullptr && peer->getVirtualWidget() == nullptr && peer->getWidgetHost() == nullptr;
}

// Embedded natives are windowed directly on the top-level; their rects must
// be translated between logical parent space and top-level space.
Point hostOffset(const Widget* widget)
{
	IWidget* peer = widget->getIWidget();
	if (!isNativeEmbedded(peer))
		return Point(0, 0);

	const Widget* parent = widget->getParent();
	if (parent == nullptr || parent->getIWidget() == nullptr)
		return Point(0, 0);

	VirtualWidget* parentVirtual = parent->getIWidget()->getVirtualWidget();
	return (parentVirtual != nullptr) ? parentVirtual->toHost(Point(0, 0)) : Point(0, 0);
}

// Shift embedded native descendants when a virtual ancestor moves; their
// windows hold top-level space positions which do not follow virtual parents.
void offsetNativeDescendants(Widget* widget, const Size& delta)
{
	for (Widget* child = widget->getFirstChild(); child != nullptr; child = child->getNextSibling())
	{
		IWidget* peer = child->getIWidget();
		if (peer == nullptr)
			continue;
		if (peer->getVirtualWidget() != nullptr)
			offsetNativeDescendants(child, delta);
		else if (isNativeEmbedded(peer))
			peer->setRect(peer->getRect().offset(delta.cx, delta.cy));
	}
}

}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Widget", Widget, EventSubject)

Widget::~Widget()
{
	T_FATAL_ASSERT(!m_widget);
}

bool Widget::create(Widget* parent, uint32_t style)
{
	if (!m_widget)
	{
		IWidgetFactory* widgetFactory = Application::getInstance()->getWidgetFactory();

		IUserWidget* widget = nullptr;
		if ((style & WsNative) != 0)
			widget = widgetFactory->createUserWidget(this);
		else if (parent->getIWidget() != nullptr && parent->getIWidget()->getWidgetHost() != nullptr)
			widget = new VirtualWidget(this);
		if (!widget)
		{
			log::error << L"Failed to create native widget peer (UserWidget)" << Endl;
			return false;
		}

		if (!widget->create(parent->getIWidget(), style))
		{
			log::error << L"Failed to create widget peer of " << type_name(this) << Endl;
			widget->destroy();
			return false;
		}

		m_widget = widget;
	}

	link(parent);

	const StyleSheet* ss = getStyleSheet();

	// Replace system default font from style sheet.
	const std::wstring fontName = ss->getValue(L"font-name");
	if (!fontName.empty())
	{
		const Unit defaultSize = getFont().getSize();
		Font font(fontName, defaultSize);

		const std::wstring fontSize = ss->getValue(L"font-size");
		if (!fontSize.empty())
			font.setSize(Unit(parseString< int32_t >(fontSize, defaultSize.get())));

		setFont(font);
	}

#if defined(_DEBUG)
	// Highlight widget with focus.
	addEventHandler< OverlayPaintEvent >([=, this](OverlayPaintEvent* event) {
		Canvas& canvas = event->getCanvas();
		if (hasFocus())
		{
			canvas.setForeground(Color4ub(0, 255, 0, 255));
			canvas.drawRect(getInnerRect().inflate(-1, -1));
		}
	});
	addEventHandler< FocusEvent >([=, this](FocusEvent* event) {
		update();
	});
#endif

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

	T_ASSERT(!m_firstChild);
	T_ASSERT(!m_lastChild);

	unlink();

	safeDestroy(m_widget);
}

void Widget::setText(const std::wstring& text)
{
	T_ASSERT(m_widget);
	m_widget->setText(text);
}

std::wstring Widget::getText() const
{
	T_ASSERT(m_widget);
	return m_widget->getText();
}

void Widget::setForeground()
{
	T_ASSERT(m_widget);
	m_widget->setForeground();
}

bool Widget::isForeground() const
{
	T_ASSERT(m_widget);
	
	if (m_widget->isForeground())
		return true;

	for (Widget* parent = m_parent; parent != nullptr; parent = parent->getParent())
		if (parent->isForeground())
			return true;

	return false;
}

void Widget::setVisible(bool visible)
{
	T_ASSERT(m_widget);
	m_visible = visible;

	if (isNativeEmbedded(m_widget))
	{
		// Embedded natives are not hidden by their virtual ancestors' windows;
		// gate effective visibility on the whole virtual chain.
		m_widget->setVisible(visible && virtualAncestorsVisible(this));
	}
	else
	{
		m_widget->setVisible(visible);
		if (m_widget->getVirtualWidget() != nullptr)
			syncNativeVisibility(this);
	}
}

bool Widget::isVisible(bool includingParents) const
{
	T_ASSERT(m_widget);

	if (!m_widget->isVisible())
		return false;

	if (m_parent != nullptr && includingParents)
	{
		if (!m_parent->isVisible(true))
			return false;
	}

	return true;
}

void Widget::setEnable(bool enable)
{
	T_ASSERT(m_widget);
	m_widget->setEnable(enable);
}

bool Widget::isEnable(bool includingParents) const
{
	T_ASSERT(m_widget);
	
	if (!m_widget->isEnable())
		return false;

	if (m_parent != nullptr && includingParents)
	{
		if (!m_parent->isEnable(true))
			return false;
	}

	return true;
}

bool Widget::hasFocus() const
{
	T_ASSERT(m_widget);
	return m_widget->hasFocus();
}

bool Widget::containFocus() const
{
	T_ASSERT(m_widget);

	if (m_widget->hasFocus())
		return true;

	for (const Widget* child = m_firstChild; child != nullptr; child = child->getNextSibling())
	{
		if (child->containFocus())
			return true;
	}

	return false;
}

void Widget::setFocus()
{
	T_ASSERT(m_widget);
	m_widget->setFocus();
}

void Widget::setRect(const Rect& rect)
{
	T_ASSERT(m_widget);

	if (m_widget->getVirtualWidget() != nullptr)
	{
		// Shift embedded natives before the peer raises the size event; a
		// layout running in that event recomputes them from the new chain,
		// harmlessly overwriting, while a pure move runs no layout at all.
		const Size delta = rect.getTopLeft() - m_widget->getRect().getTopLeft();
		if (delta.cx != 0 || delta.cy != 0)
			offsetNativeDescendants(this, delta);
		m_widget->setRect(rect);
	}
	else
	{
		const Point offset = hostOffset(this);
		m_widget->setRect(rect.offset(offset.x, offset.y));
	}
}

Rect Widget::getRect() const
{
	T_ASSERT(m_widget);
	const Point offset = hostOffset(this);
	return m_widget->getRect().offset(-offset.x, -offset.y);
}

Rect Widget::getInnerRect() const
{
	T_ASSERT(m_widget);
	return m_widget->getInnerRect();
}

Rect Widget::getNormalRect() const
{
	T_ASSERT(m_widget);
	return m_widget->getNormalRect();
}

void Widget::setFont(const Font& font)
{
	T_ASSERT(m_widget);
	m_widget->setFont(font);
}

Font Widget::getFont() const
{
	T_ASSERT(m_widget);
	return Font(m_widget->getFont());
}

FontMetric Widget::getFontMetric() const
{
	T_ASSERT(m_widget);
	return FontMetric(m_widget->getFontMetric());
}

void Widget::setCursor(Cursor cursor)
{
	T_ASSERT(m_widget);
	m_widget->setCursor(cursor);
}

void Widget::resetCursor()
{
	T_ASSERT(m_widget);
	m_widget->setCursor(Cursor::Arrow);
}

void Widget::update(const Rect* rc, bool immediate)
{
	T_ASSERT(m_widget);
	m_widget->update(rc, immediate);
}

void Widget::show()
{
	setVisible(true);
}

void Widget::hide()
{
	setVisible(false);
}

int32_t Widget::dpi() const
{
	T_ASSERT(m_widget);
	return m_widget->dpi96(96);
}

int32_t Widget::pixel(Unit measure) const
{
	T_ASSERT(m_widget);
	return m_widget->dpi96(measure.get());
}

Unit Widget::unit(int32_t measure) const
{
	T_ASSERT(m_widget);
	return Unit(m_widget->invdpi96(measure));
}

Point Widget::pixel(const UnitPoint& measure) const
{
	return Point(pixel(measure.x), pixel(measure.y));
}

UnitPoint Widget::unit(const Point& pt) const
{
	return UnitPoint(unit(pt.x), unit(pt.y));
}

Size Widget::pixel(const UnitSize& measure) const
{
	return Size(pixel(measure.cx), pixel(measure.cy));
}

UnitSize Widget::unit(const Size& sz) const
{
	return UnitSize(unit(sz.cx), unit(sz.cy));
}

Rect Widget::pixel(const UnitRect& measure) const
{
	return Rect(pixel(measure.getTopLeft()), pixel(measure.getBottomRight()));
}

UnitRect Widget::unit(const Rect& rc) const
{
	return UnitRect(unit(rc.getTopLeft()), unit(rc.getBottomRight()));
}

bool Widget::hasCapture() const
{
	T_ASSERT(m_widget);
	return m_widget->hasCapture();
}

bool Widget::hasCaptureOther() const
{
	if (m_widget == nullptr)
		return false;

	ITopLevelWidgetHost* host = m_widget->getWidgetHost();
	if (host == nullptr)
		return false;

	IWidget* capture = host->getCaptureWidget();
	return capture != nullptr && capture != m_widget;
}

void Widget::setCapture()
{
	T_ASSERT(m_widget);
	m_widget->setCapture();
}

void Widget::releaseCapture()
{
	T_ASSERT(m_widget);
	m_widget->releaseCapture();
}

void Widget::startTimer(int interval)
{
	T_ASSERT(m_widget);
	m_widget->startTimer(interval);
}

void Widget::stopTimer()
{
	T_ASSERT(m_widget);
	m_widget->stopTimer();
}

Point Widget::getMousePosition(bool relative) const
{
	T_ASSERT(m_widget);
	return m_widget->getMousePosition(relative);
}

Point Widget::screenToClient(const Point& pt) const
{
	T_ASSERT(m_widget);
	return m_widget->screenToClient(pt);
}

Point Widget::clientToScreen(const Point& pt) const
{
	T_ASSERT(m_widget);
	return m_widget->clientToScreen(pt);
}

bool Widget::hitTest(const Point& pt) const
{
	T_ASSERT(m_widget);
	return m_widget->hitTest(pt);
}

void Widget::setChildRects(const WidgetRect* childRects, uint32_t count, bool redraw)
{
	T_ASSERT(m_widget);

	// Anything virtual goes through Widget::setRect so host space translation
	// and native descendant repositioning apply; only native children of
	// native widgets benefit from the batched path.
	StaticVector< IWidgetRect, 32 > native;
	for (uint32_t i = 0; i < count; ++i)
	{
		Widget* child = childRects[i].widget;
		if (child == nullptr || child->getIWidget() == nullptr)
			continue;

		if (m_widget->getVirtualWidget() != nullptr || child->getIWidget()->getVirtualWidget() != nullptr)
		{
			child->setRect(childRects[i].rect);
			continue;
		}

		if (native.full())
		{
			m_widget->setChildRects(native.c_ptr(), (uint32_t)native.size(), redraw);
			native.clear();
		}
		native.push_back({ child->getIWidget(), childRects[i].rect });
	}
	if (!native.empty())
		m_widget->setChildRects(native.c_ptr(), (uint32_t)native.size(), redraw);
}

Size Widget::getMinimumSize() const
{
	T_ASSERT(m_widget);
	return m_widget->getMinimumSize();
}

Size Widget::getPreferredSize(const Size& hint) const
{
	T_ASSERT(m_widget);
	return m_widget->getPreferredSize(hint);
}

Size Widget::getMaximumSize() const
{
	T_ASSERT(m_widget);
	return m_widget->getMaximumSize();
}

void Widget::setHorizontalAlign(Align halign)
{
	m_halign = halign;
}

Align Widget::getHorizontalAlign() const
{
	return m_halign;
}

void Widget::setVerticalAlign(Align valign)
{
	m_valign = valign;
}

Align Widget::getVerticalAlign() const
{
	return m_valign;
}

void Widget::setStyleSheet(const StyleSheet* styleSheet)
{
	m_styleSheet = styleSheet;
}

const StyleSheet* Widget::getStyleSheet() const
{
	if (m_styleSheet)
		return m_styleSheet;

	if (m_parent)
		return m_parent->getStyleSheet();
	else
		return Application::getInstance()->getStyleSheet();
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
		m_nextSibling = nullptr;
		if (parent->m_lastChild != nullptr)
			parent->m_lastChild->m_nextSibling = this;
		else
			parent->m_firstChild = this;
		parent->m_lastChild = this;

		// Invoke event on parent and it's ancestors.
		Widget* ancestor = m_parent;
		do
		{
			ChildEvent childEvent(ancestor, m_parent, this, true);
			ancestor->raiseEvent(&childEvent);
			ancestor = ancestor->getParent();
		}
		while (ancestor != nullptr);
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

	// Invoke event on parent and it's ancestors.
	Widget* ancestor = m_parent;
	while (ancestor != nullptr)
	{
		ChildEvent childEvent(ancestor, m_parent, this, false);
		ancestor->raiseEvent(&childEvent);
		ancestor = ancestor->getParent();
	}

	m_parent = nullptr;
	m_nextSibling = nullptr;
	m_previousSibling = nullptr;
}

void Widget::setParent(Widget* parent)
{
	// Embedded natives derive position and visibility from the virtual chain
	// which is about to change; capture logical rects before reparenting.
	AlignedVector< std::pair< Widget*, Rect > > embedded;
	collectEmbeddedNativeRects(this, embedded);

	unlink();
	if (parent)
	{
		m_widget->setParent(parent->getIWidget());
		link(parent);

		for (auto& it : embedded)
			it.first->setRect(it.second);

		if (m_widget->getVirtualWidget() != nullptr)
			syncNativeVisibility(this);
		else if (isNativeEmbedded(m_widget))
			m_widget->setVisible(m_visible && virtualAncestorsVisible(this));
	}
	else
		m_widget->setParent(nullptr);
}

Widget* Widget::getParent() const
{
	return m_parent;
}

Widget* Widget::getAncestor() const
{
	Widget* ancestor = m_parent;
	while (ancestor && ancestor->m_parent)
		ancestor = ancestor->m_parent;
	return ancestor;
}

Widget* Widget::getPreviousSibling() const
{
	return m_previousSibling;
}

Widget* Widget::getNextSibling() const
{
	return m_nextSibling;
}

Widget* Widget::getFirstChild() const
{
	return m_firstChild;
}

Widget* Widget::getLastChild() const
{
	return m_lastChild;
}

IWidget* Widget::getIWidget() const
{
	return m_widget;
}

bool Widget::virtualAncestorsVisible(const Widget* widget)
{
	for (const Widget* parent = widget->m_parent; parent != nullptr; parent = parent->m_parent)
	{
		IWidget* peer = parent->getIWidget();
		if (peer == nullptr || peer->getVirtualWidget() == nullptr)
			break;
		if (!parent->m_visible)
			return false;
	}
	return true;
}

void Widget::syncNativeVisibility(Widget* widget)
{
	for (Widget* child = widget->m_firstChild; child != nullptr; child = child->m_nextSibling)
	{
		IWidget* peer = child->getIWidget();
		if (peer == nullptr)
			continue;
		if (peer->getVirtualWidget() != nullptr)
			syncNativeVisibility(child);
		else if (isNativeEmbedded(peer))
			peer->setVisible(child->m_visible && virtualAncestorsVisible(child));
	}
}

void Widget::collectEmbeddedNativeRects(Widget* widget, AlignedVector< std::pair< Widget*, Rect > >& outRects)
{
	IWidget* peer = widget->getIWidget();
	if (peer == nullptr)
		return;

	if (isNativeEmbedded(peer))
	{
		outRects.push_back({ widget, widget->getRect() });
		return;
	}

	// Top-level subtrees position themselves in screen space; leave them.
	if (peer->getVirtualWidget() == nullptr)
		return;

	for (Widget* child = widget->m_firstChild; child != nullptr; child = child->m_nextSibling)
		collectEmbeddedNativeRects(child, outRects);
}

}
