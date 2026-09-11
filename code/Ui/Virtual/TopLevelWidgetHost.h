/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Ui/Itf/ITopLevelWidgetHost.h"

namespace traktor::ui
{

class VirtualWidget;

/*! Router owned by a native top-level peer; paints and dispatches input
 * to the tree of virtual widgets hosted inside that top-level window.
 * \ingroup UI
 */
class TopLevelWidgetHost : public RefCountImpl< ITopLevelWidgetHost >
{
public:
	explicit TopLevelWidgetHost(ITopLevelWidgetHost::IPeer* peer, EventSubject* owner);

	// ITopLevelWidgetHost

	virtual void paint(Canvas& canvas, const Rect& rcUpdate) override final;

	virtual bool dispatchMouseButtonDown(int32_t button, const Point& pt) override final;

	virtual bool dispatchMouseButtonUp(int32_t button, const Point& pt) override final;

	virtual bool dispatchMouseDoubleClick(int32_t button, const Point& pt) override final;

	virtual bool dispatchMouseMove(int32_t button, const Point& pt) override final;

	virtual bool dispatchMouseWheel(int32_t rotation, const Point& pt) override final;

	virtual bool dispatchKeyDown(VirtualKey vk, int32_t systemKey) override final;

	virtual bool dispatchKeyUp(VirtualKey vk, int32_t systemKey) override final;

	virtual bool dispatchKey(VirtualKey vk, int32_t systemKey, wchar_t ch) override final;

	virtual void dispatchMouseTrack(bool entered, const Point& pt) override final;

	virtual void dispatchFocus(bool gained) override final;

	virtual void captureLost() override final;

	virtual Point getLastMousePosition() const override final;

	virtual IWidget* getCaptureWidget() const override final;

	/*! \name Services for virtual widgets. */
	//@{

	ITopLevelWidgetHost::IPeer* getPeer() const;

	EventSubject* getOwner() const;

	void addChild(VirtualWidget* child);

	void removeChild(VirtualWidget* child);

	AlignedVector< VirtualWidget* >& getChildren();

	void requestUpdate(const Rect& rc, bool immediate);

	void setFocus(VirtualWidget* widget);

	VirtualWidget* getFocus() const;

	void setCapture(VirtualWidget* widget);

	void releaseCapture(VirtualWidget* widget);

	VirtualWidget* getCapture() const;

	VirtualWidget* getHover() const;

	/*! Drop every reference to widget; must be called before it is deleted. */
	void detach(VirtualWidget* widget);

	//@}

	VirtualWidget* hitTest(const Point& pt) const;

private:
	ITopLevelWidgetHost::IPeer* m_peer;
	EventSubject* m_owner;
	AlignedVector< VirtualWidget* > m_children;
	VirtualWidget* m_hover = nullptr;
	VirtualWidget* m_capture = nullptr;
	VirtualWidget* m_focus = nullptr;
	VirtualWidget* m_lastDown = nullptr;
	Point m_lastMousePosition;
	bool m_inside = false;
	bool m_inPaint = false;

	void paintWidget(Canvas& canvas, VirtualWidget* widget, const Rect& clip, const Point& offset);

	void updateHover(const Point& pt);

	void applyCursor();
};

}
