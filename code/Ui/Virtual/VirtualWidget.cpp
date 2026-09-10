/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Core/Log/Log.h"
#include "Ui/EventSubject.h"
#include "Ui/Events/AllEvents.h"
#include "Ui/Itf/IFontMetricProvider.h"
#include "Ui/Virtual/VirtualWidget.h"

namespace traktor::ui
{

VirtualWidget::VirtualWidget(EventSubject* owner)
:	m_owner(owner)
{
}

bool VirtualWidget::create(IWidget* parent, int style)
{
	if (parent == nullptr)
		return false;

	// Hosts are created solely by createTopLevelWidgetHost; downcast is safe.
	m_host = static_cast< TopLevelWidgetHost* >(parent->getWidgetHost());
	if (m_host == nullptr)
		return false;

	m_style = (uint32_t)style;
	m_font = parent->getFont();

	m_parent = parent->getVirtualWidget();
	if (m_parent != nullptr)
		m_parent->m_children.push_back(this);
	else
		m_host->addChild(this);

	return true;
}

void VirtualWidget::destroy()
{
	stopTimer();

	if (m_host != nullptr)
	{
		m_host->detach(this);

		if (m_visible && m_rect.area() > 0)
			m_host->requestUpdate(getHostRect(), false);

		if (m_parent != nullptr)
		{
			auto& siblings = m_parent->m_children;
			auto it = std::find(siblings.begin(), siblings.end(), this);
			if (it != siblings.end())
				siblings.erase(it);
		}
		else
			m_host->removeChild(this);
	}

	// Ui layer destroys widgets leaf first; a surviving child here is a bug.
	T_FATAL_ASSERT(m_children.empty());

	m_parent = nullptr;
	m_host = nullptr;

	delete this;
}

void VirtualWidget::setParent(IWidget* parent)
{
	if (m_host != nullptr && m_visible && m_rect.area() > 0)
		m_host->requestUpdate(getHostRect(), false);

	if (m_parent != nullptr)
	{
		auto& siblings = m_parent->m_children;
		auto it = std::find(siblings.begin(), siblings.end(), this);
		if (it != siblings.end())
			siblings.erase(it);
	}
	else if (m_host != nullptr)
		m_host->removeChild(this);

	m_parent = nullptr;

	if (parent == nullptr)
		return;

	TopLevelWidgetHost* host = static_cast< TopLevelWidgetHost* >(parent->getWidgetHost());
	if (host == nullptr)
	{
		// Reparenting a virtual widget under a native, non top-level widget is
		// not supported; leave orphaned rather than crash.
		log::error << L"Virtual widget " << type_name(m_owner) << L" reparented under host-less native widget; orphaned." << Endl;
		return;
	}

	if (host != m_host)
		migrateHost(host);

	m_parent = parent->getVirtualWidget();
	if (m_parent != nullptr)
		m_parent->m_children.push_back(this);
	else
		m_host->addChild(this);

	if (m_visible && m_rect.area() > 0)
		m_host->requestUpdate(getHostRect(), false);
}

void VirtualWidget::setText(const std::wstring& text)
{
	m_text = text;
}

std::wstring VirtualWidget::getText() const
{
	return m_text;
}

void VirtualWidget::setForeground()
{
}

bool VirtualWidget::isForeground() const
{
	return false;
}

void VirtualWidget::setVisible(bool visible)
{
	if (visible == m_visible)
		return;

	m_visible = visible;

	if (m_host != nullptr && m_rect.area() > 0)
		m_host->requestUpdate(getHostRect(), false);

	ShowEvent showEvent(m_owner, visible);
	m_owner->raiseEvent(&showEvent);
}

bool VirtualWidget::isVisible() const
{
	return m_visible;
}

void VirtualWidget::setEnable(bool enable)
{
	m_enable = enable;
}

bool VirtualWidget::isEnable() const
{
	return m_enable;
}

bool VirtualWidget::hasFocus() const
{
	return m_host != nullptr && m_host->getFocus() == this;
}

void VirtualWidget::setFocus()
{
	if (m_host != nullptr)
		m_host->setFocus(this);
}

bool VirtualWidget::hasCapture() const
{
	return m_host != nullptr && m_host->getCapture() == this;
}

void VirtualWidget::setCapture()
{
	if (m_host != nullptr)
		m_host->setCapture(this);
}

void VirtualWidget::releaseCapture()
{
	if (m_host != nullptr)
		m_host->releaseCapture(this);
}

void VirtualWidget::startTimer(int interval)
{
	stopTimer();

	m_timerInterval = interval;
	m_timer = m_host->getPeer()->startHostTimer(interval, [this]() {
		if (!m_visible)
			return;
		TimerEvent timerEvent(m_owner);
		m_owner->raiseEvent(&timerEvent);
	});
}

void VirtualWidget::stopTimer()
{
	if (m_timer >= 0)
	{
		m_host->getPeer()->stopHostTimer(m_timer);
		m_timer = -1;
		m_timerInterval = 0;
	}
}

void VirtualWidget::setRect(const Rect& rect)
{
	if (rect == m_rect)
		return;

	const Rect rcOldHost = getHostRect();
	const bool moved = (rect.getTopLeft() != m_rect.getTopLeft());
	const bool sized = (rect.getSize() != m_rect.getSize());

	m_rect = rect;

	if (m_host != nullptr && m_visible)
	{
		if (rcOldHost.area() > 0)
			m_host->requestUpdate(rcOldHost, false);
		if (m_rect.area() > 0)
			m_host->requestUpdate(getHostRect(), false);
	}

	if (moved)
	{
		MoveEvent moveEvent(m_owner, m_rect.getTopLeft());
		m_owner->raiseEvent(&moveEvent);
	}
	if (sized)
	{
		SizeEvent sizeEvent(m_owner, m_rect.getSize());
		m_owner->raiseEvent(&sizeEvent);
	}
}

Rect VirtualWidget::getRect() const
{
	return m_rect;
}

Rect VirtualWidget::getInnerRect() const
{
	return Rect(0, 0, m_rect.getWidth(), m_rect.getHeight());
}

Rect VirtualWidget::getNormalRect() const
{
	return Rect(0, 0, m_rect.getWidth(), m_rect.getHeight());
}

void VirtualWidget::setFont(const Font& font)
{
	m_font = font;
}

Font VirtualWidget::getFont() const
{
	return m_font;
}

const IFontMetric* VirtualWidget::getFontMetric() const
{
	return this;
}

void VirtualWidget::setCursor(Cursor cursor)
{
	m_cursor = cursor;
	if (m_host != nullptr && (m_host->getHover() == this || m_host->getCapture() == this))
		m_host->getPeer()->getPeerWidget()->setCursor(cursor);
}

Point VirtualWidget::getMousePosition(bool relative) const
{
	const Point pt = m_host->getLastMousePosition();
	if (relative)
		return fromHost(pt);
	else
		return m_host->getPeer()->getPeerWidget()->clientToScreen(pt);
}

Point VirtualWidget::screenToClient(const Point& pt) const
{
	return fromHost(m_host->getPeer()->getPeerWidget()->screenToClient(pt));
}

Point VirtualWidget::clientToScreen(const Point& pt) const
{
	return m_host->getPeer()->getPeerWidget()->clientToScreen(toHost(pt));
}

bool VirtualWidget::hitTest(const Point& pt) const
{
	return getInnerRect().inside(screenToClient(pt));
}

void VirtualWidget::setChildRects(const IWidgetRect* childRects, uint32_t count, bool redraw)
{
	for (uint32_t i = 0; i < count; ++i)
	{
		if (childRects[i].widget != nullptr)
			childRects[i].widget->setRect(childRects[i].rect);
	}
}

Size VirtualWidget::getMinimumSize() const
{
	return Size(0, 0);
}

Size VirtualWidget::getPreferredSize(const Size& hint) const
{
	return Size(0, 0);
}

Size VirtualWidget::getMaximumSize() const
{
	return Size(65535, 65535);
}

void VirtualWidget::update(const Rect* rc, bool immediate)
{
	if (m_host == nullptr || !isEffectivelyVisible() || m_rect.area() <= 0)
		return;

	const Point origin = toHost(Point(0, 0));
	const Rect rcHost = (rc != nullptr) ? rc->offset(origin.x, origin.y) : getHostRect();

	m_host->requestUpdate(rcHost, immediate);
}

int32_t VirtualWidget::dpi96(int32_t measure) const
{
	return m_host->getPeer()->getPeerWidget()->dpi96(measure);
}

int32_t VirtualWidget::invdpi96(int32_t measure) const
{
	return m_host->getPeer()->getPeerWidget()->invdpi96(measure);
}

void* VirtualWidget::getInternalHandle()
{
	return m_host->getPeer()->getPeerWidget()->getInternalHandle();
}

SystemWindow VirtualWidget::getSystemWindow()
{
	T_FATAL_ASSERT_M(false, L"Virtual widgets cannot host system windows; create widget with WsNative.");
	return SystemWindow();
}

ITopLevelWidgetHost* VirtualWidget::getWidgetHost()
{
	return m_host;
}

VirtualWidget* VirtualWidget::getVirtualWidget()
{
	return this;
}

void VirtualWidget::getAscentAndDescent(int32_t& outAscent, int32_t& outDescent) const
{
	const IFontMetricProvider* provider = getProvider();
	if (provider != nullptr)
		provider->getAscentAndDescent(m_font, outAscent, outDescent);
	else
	{
		outAscent = 0;
		outDescent = 0;
	}
}

int32_t VirtualWidget::getAdvance(wchar_t ch, wchar_t next) const
{
	const IFontMetricProvider* provider = getProvider();
	return (provider != nullptr) ? provider->getAdvance(m_font, ch, next) : 0;
}

int32_t VirtualWidget::getLineSpacing() const
{
	const IFontMetricProvider* provider = getProvider();
	return (provider != nullptr) ? provider->getLineSpacing(m_font) : 0;
}

Size VirtualWidget::getExtent(const std::wstring& text) const
{
	const IFontMetricProvider* provider = getProvider();
	return (provider != nullptr) ? provider->getExtent(m_font, text) : Size(0, 0);
}

Point VirtualWidget::toHost(const Point& pt) const
{
	Point result = pt;
	for (const VirtualWidget* widget = this; widget != nullptr; widget = widget->m_parent)
	{
		result.x += widget->m_rect.left;
		result.y += widget->m_rect.top;
	}
	return result;
}

Point VirtualWidget::fromHost(const Point& pt) const
{
	Point result = pt;
	for (const VirtualWidget* widget = this; widget != nullptr; widget = widget->m_parent)
	{
		result.x -= widget->m_rect.left;
		result.y -= widget->m_rect.top;
	}
	return result;
}

Rect VirtualWidget::getHostRect() const
{
	return Rect(toHost(Point(0, 0)), m_rect.getSize());
}

bool VirtualWidget::isEffectivelyVisible() const
{
	for (const VirtualWidget* widget = this; widget != nullptr; widget = widget->m_parent)
	{
		if (!widget->m_visible)
			return false;
	}
	return true;
}

bool VirtualWidget::isEffectivelyEnabled() const
{
	for (const VirtualWidget* widget = this; widget != nullptr; widget = widget->m_parent)
	{
		if (!widget->m_enable)
			return false;
	}
	return true;
}

const IFontMetricProvider* VirtualWidget::getProvider() const
{
	return (m_host != nullptr) ? m_host->getPeer()->getPeerWidget()->getFontMetricProvider() : nullptr;
}

void VirtualWidget::migrateHost(TopLevelWidgetHost* host)
{
	const int32_t interval = m_timerInterval;
	stopTimer();

	if (m_host != nullptr)
		m_host->detach(this);

	m_host = host;

	if (interval > 0)
		startTimer(interval);

	for (auto child : m_children)
		child->migrateHost(host);
}

}
