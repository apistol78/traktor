/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Ui/Canvas.h"
#include "Ui/EventSubject.h"
#include "Ui/Events/AllEvents.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Virtual/TopLevelWidgetHost.h"
#include "Ui/Virtual/VirtualWidget.h"

namespace traktor::ui
{
namespace
{

Rect intersection(const Rect& a, const Rect& b)
{
	return Rect(
		std::max(a.left, b.left),
		std::max(a.top, b.top),
		std::min(a.right, b.right),
		std::min(a.bottom, b.bottom)
	);
}

// Half-open containment; a point on a shared edge belongs to one widget only.
bool inside(const Rect& rc, const Point& pt)
{
	return pt.x >= rc.left && pt.x < rc.right && pt.y >= rc.top && pt.y < rc.bottom;
}

VirtualWidget* hitTestChildren(const AlignedVector< VirtualWidget* >& children, const Point& pt, const Point& offset)
{
	for (auto it = children.rbegin(); it != children.rend(); ++it)
	{
		VirtualWidget* child = *it;
		if (!child->isVisible())
			continue;

		const Rect rc = child->getRect().offset(offset.x, offset.y);
		if (!inside(rc, pt))
			continue;

		VirtualWidget* hit = hitTestChildren(child->getChildren(), pt, rc.getTopLeft());
		return (hit != nullptr) ? hit : child;
	}
	return nullptr;
}

}

TopLevelWidgetHost::TopLevelWidgetHost(ITopLevelWidgetHost::IPeer* peer, EventSubject* owner)
:	m_peer(peer)
,	m_owner(owner)
{
}

void TopLevelWidgetHost::paint(Canvas& canvas, const Rect& rcUpdate)
{
	m_inPaint = true;

	canvas.setBaseClip(rcUpdate);

	PaintEvent paintEvent(m_owner, canvas, rcUpdate);
	m_owner->raiseEvent(&paintEvent);

	OverlayPaintEvent overlayPaintEvent(m_owner, canvas, rcUpdate);
	m_owner->raiseEvent(&overlayPaintEvent);

	for (auto child : m_children)
		paintWidget(canvas, child, rcUpdate, Point(0, 0));

	canvas.setOrigin(Point(0, 0));
	canvas.clearBaseClip();

	m_inPaint = false;
}

bool TopLevelWidgetHost::dispatchMouseButtonDown(int32_t button, const Point& pt)
{
	m_lastMousePosition = pt;

	VirtualWidget* target = (m_capture != nullptr) ? m_capture : hitTest(pt);
	m_lastDown = target;

	if (target == nullptr)
	{
		MouseButtonDownEvent event(m_owner, button, pt);
		m_owner->raiseEvent(&event);
		return event.consumed();
	}

	if (!target->isEffectivelyEnabled())
		return true;

	if ((target->getStyle() & WsFocus) != 0)
		setFocus(target);

	MouseButtonDownEvent event(target->getOwner(), button, target->fromHost(pt));
	target->getOwner()->raiseEvent(&event);
	return event.consumed();
}

bool TopLevelWidgetHost::dispatchMouseButtonUp(int32_t button, const Point& pt)
{
	m_lastMousePosition = pt;

	VirtualWidget* target = (m_capture != nullptr) ? m_capture : hitTest(pt);
	if (target == nullptr)
	{
		MouseButtonUpEvent event(m_owner, button, pt);
		m_owner->raiseEvent(&event);
		return event.consumed();
	}

	if (!target->isEffectivelyEnabled())
		return true;

	MouseButtonUpEvent event(target->getOwner(), button, target->fromHost(pt));
	target->getOwner()->raiseEvent(&event);
	return event.consumed();
}

bool TopLevelWidgetHost::dispatchMouseDoubleClick(int32_t button, const Point& pt)
{
	m_lastMousePosition = pt;

	VirtualWidget* target = (m_capture != nullptr) ? m_capture : hitTest(pt);

	// Second click landed on another widget; deliver as an ordinary press.
	if (target != m_lastDown)
		return dispatchMouseButtonDown(button, pt);

	if (target == nullptr)
	{
		MouseDoubleClickEvent event(m_owner, button, pt);
		m_owner->raiseEvent(&event);
		return event.consumed();
	}

	if (!target->isEffectivelyEnabled())
		return true;

	MouseDoubleClickEvent event(target->getOwner(), button, target->fromHost(pt));
	target->getOwner()->raiseEvent(&event);
	return event.consumed();
}

bool TopLevelWidgetHost::dispatchMouseMove(int32_t button, const Point& pt)
{
	m_lastMousePosition = pt;

	if (m_capture == nullptr)
		updateHover(pt);

	VirtualWidget* target = (m_capture != nullptr) ? m_capture : m_hover;
	applyCursor();

	if (target == nullptr)
	{
		MouseMoveEvent event(m_owner, button, pt);
		m_owner->raiseEvent(&event);
		return event.consumed();
	}

	if (!target->isEffectivelyEnabled())
		return true;

	MouseMoveEvent event(target->getOwner(), button, target->fromHost(pt));
	target->getOwner()->raiseEvent(&event);
	return event.consumed();
}

bool TopLevelWidgetHost::dispatchMouseWheel(int32_t rotation, const Point& pt)
{
	VirtualWidget* target = (m_hover != nullptr) ? m_hover : m_focus;

	// Wheel events carry screen coordinates on every backend.
	const Point ptScreen = m_peer->getPeerWidget()->clientToScreen(pt);

	if (target == nullptr)
	{
		MouseWheelEvent event(m_owner, rotation, ptScreen);
		m_owner->raiseEvent(&event);
		return event.consumed();
	}

	if (!target->isEffectivelyEnabled())
		return true;

	MouseWheelEvent event(target->getOwner(), rotation, ptScreen);
	target->getOwner()->raiseEvent(&event);
	return event.consumed();
}

bool TopLevelWidgetHost::dispatchKeyDown(VirtualKey vk, int32_t systemKey)
{
	EventSubject* target = (m_focus != nullptr) ? m_focus->getOwner() : m_owner;
	KeyDownEvent event(target, vk, systemKey, 0);
	target->raiseEvent(&event);
	return event.consumed();
}

bool TopLevelWidgetHost::dispatchKeyUp(VirtualKey vk, int32_t systemKey)
{
	EventSubject* target = (m_focus != nullptr) ? m_focus->getOwner() : m_owner;
	KeyUpEvent event(target, vk, systemKey, 0, false);
	target->raiseEvent(&event);
	return event.consumed();
}

bool TopLevelWidgetHost::dispatchKey(VirtualKey vk, int32_t systemKey, wchar_t ch)
{
	EventSubject* target = (m_focus != nullptr) ? m_focus->getOwner() : m_owner;
	KeyEvent event(target, vk, systemKey, ch);
	target->raiseEvent(&event);
	return event.consumed();
}

void TopLevelWidgetHost::dispatchMouseTrack(bool entered, const Point& pt)
{
	m_inside = entered;

	// While a widget holds capture the pointer belongs to it. Crossing the
	// window edge, or a native child widget with a peer of its own, must not
	// hand hover to anyone else; dispatchMouseMove freezes hover for the same
	// reason. Hover is resolved again once the capture is released.
	if (m_capture != nullptr)
		return;

	if (entered)
	{
		if (m_hover == nullptr)
		{
			MouseTrackEvent event(m_owner, true);
			m_owner->raiseEvent(&event);
		}
	}
	else
	{
		EventSubject* owner = (m_hover != nullptr) ? m_hover->getOwner() : m_owner;
		m_hover = nullptr;
		MouseTrackEvent event(owner, false);
		owner->raiseEvent(&event);
	}
}

void TopLevelWidgetHost::dispatchFocus(bool gained)
{
	EventSubject* target = (m_focus != nullptr) ? m_focus->getOwner() : m_owner;
	FocusEvent event(target, gained);
	target->raiseEvent(&event);
}

void TopLevelWidgetHost::captureLost()
{
	// Genuine loss, e.g. another window took the capture; synthesize a button
	// release so the widget's internal drag state doesn't get stuck.
	VirtualWidget* lost = m_capture;
	m_capture = nullptr;
	if (lost != nullptr)
	{
		MouseButtonUpEvent event(lost->getOwner(), MbtLeft, lost->fromHost(m_lastMousePosition));
		lost->getOwner()->raiseEvent(&event);
	}

	updateHover(m_lastMousePosition);
}

ITopLevelWidgetHost::IPeer* TopLevelWidgetHost::getPeer() const
{
	return m_peer;
}

EventSubject* TopLevelWidgetHost::getOwner() const
{
	return m_owner;
}

void TopLevelWidgetHost::addChild(VirtualWidget* child)
{
	m_children.push_back(child);
}

void TopLevelWidgetHost::removeChild(VirtualWidget* child)
{
	auto it = std::find(m_children.begin(), m_children.end(), child);
	if (it != m_children.end())
		m_children.erase(it);
}

AlignedVector< VirtualWidget* >& TopLevelWidgetHost::getChildren()
{
	return m_children;
}

void TopLevelWidgetHost::requestUpdate(const Rect& rc, bool immediate)
{
	// Immediate repaint while already painting would recurse into the canvas.
	m_peer->getPeerWidget()->update(&rc, immediate && !m_inPaint);
}

void TopLevelWidgetHost::setFocus(VirtualWidget* widget)
{
	if (widget == m_focus)
		return;

	VirtualWidget* previous = m_focus;
	m_focus = widget;

	if (previous != nullptr)
	{
		FocusEvent event(previous->getOwner(), false);
		previous->getOwner()->raiseEvent(&event);
	}
	if (m_focus != nullptr)
	{
		FocusEvent event(m_focus->getOwner(), true);
		m_focus->getOwner()->raiseEvent(&event);
	}
}

VirtualWidget* TopLevelWidgetHost::getFocus() const
{
	return m_focus;
}

void TopLevelWidgetHost::setCapture(VirtualWidget* widget)
{
	m_capture = widget;
	m_peer->getPeerWidget()->setCapture();
}

void TopLevelWidgetHost::releaseCapture(VirtualWidget* widget)
{
	if (m_capture != widget)
		return;

	m_capture = nullptr;
	m_peer->getPeerWidget()->releaseCapture();

	// Hover was frozen while captured; resolve it against where the pointer
	// actually is now.
	updateHover(m_lastMousePosition);
}

IWidget* TopLevelWidgetHost::getCaptureWidget() const
{
	return m_capture;
}

VirtualWidget* TopLevelWidgetHost::getCapture() const
{
	return m_capture;
}

VirtualWidget* TopLevelWidgetHost::getHover() const
{
	return m_hover;
}

Point TopLevelWidgetHost::getLastMousePosition() const
{
	return m_lastMousePosition;
}

void TopLevelWidgetHost::detach(VirtualWidget* widget)
{
	if (m_hover == widget)
		m_hover = nullptr;
	if (m_capture == widget)
		releaseCapture(widget);
	if (m_focus == widget)
		m_focus = nullptr;
	if (m_lastDown == widget)
		m_lastDown = nullptr;
}

VirtualWidget* TopLevelWidgetHost::hitTest(const Point& pt) const
{
	return hitTestChildren(m_children, pt, Point(0, 0));
}

void TopLevelWidgetHost::paintWidget(Canvas& canvas, VirtualWidget* widget, const Rect& clip, const Point& offset)
{
	if (!widget->isVisible())
		return;

	const Rect rcHost = widget->getRect().offset(offset.x, offset.y);
	const Rect rcClip = intersection(rcHost, clip);
	if (rcClip.getWidth() <= 0 || rcClip.getHeight() <= 0)
		return;

	const Point origin = rcHost.getTopLeft();
	const Rect rcLocalUpdate = rcClip.offset(-origin.x, -origin.y);

	canvas.setOrigin(origin);
	canvas.setBaseClip(rcLocalUpdate);
	canvas.setFont(widget->getFont());
	canvas.setForeground(Color4ub(0, 0, 0, 255));
	canvas.setBackground(Color4ub(255, 255, 255, 255));

	PaintEvent paintEvent(widget->getOwner(), canvas, rcLocalUpdate);
	widget->getOwner()->raiseEvent(&paintEvent);

	OverlayPaintEvent overlayPaintEvent(widget->getOwner(), canvas, rcLocalUpdate);
	widget->getOwner()->raiseEvent(&overlayPaintEvent);

	for (auto child : widget->getChildren())
		paintWidget(canvas, child, rcClip, origin);
}

void TopLevelWidgetHost::updateHover(const Point& pt)
{
	VirtualWidget* hover = m_inside ? hitTest(pt) : nullptr;
	if (hover == m_hover)
		return;

	EventSubject* oldOwner = (m_hover != nullptr) ? m_hover->getOwner() : m_owner;
	EventSubject* newOwner = (hover != nullptr) ? hover->getOwner() : m_owner;

	m_hover = hover;

	if (oldOwner != newOwner)
	{
		MouseTrackEvent leaveEvent(oldOwner, false);
		oldOwner->raiseEvent(&leaveEvent);

		MouseTrackEvent enterEvent(newOwner, true);
		newOwner->raiseEvent(&enterEvent);
	}
}

void TopLevelWidgetHost::applyCursor()
{
	VirtualWidget* widget = (m_capture != nullptr) ? m_capture : m_hover;
	m_peer->getPeerWidget()->setCursor(widget != nullptr ? widget->getCursor() : Cursor::Arrow);
}

Ref< ITopLevelWidgetHost > createTopLevelWidgetHost(ITopLevelWidgetHost::IPeer* peer, EventSubject* owner)
{
	return new TopLevelWidgetHost(peer, owner);
}

}
