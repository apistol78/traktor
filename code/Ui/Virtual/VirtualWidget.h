/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Containers/AlignedVector.h"
#include "Core/Ref.h"
#include "Ui/Itf/IFontMetric.h"
#include "Ui/Itf/IUserWidget.h"
#include "Ui/Virtual/TopLevelWidgetHost.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

class EventSubject;
class IFontMetricProvider;

/*! Backend independent widget hosted, painted and routed by the
 * TopLevelWidgetHost of its containing native top-level window.
 * \ingroup UI
 */
class T_DLLCLASS VirtualWidget
:	public IUserWidget
,	public IFontMetric
{
public:
	explicit VirtualWidget(EventSubject* owner);

	// IUserWidget

	virtual bool create(IWidget* parent, int style) override;

	// IWidget

	virtual void destroy() override;

	virtual void setParent(IWidget* parent) override;

	virtual void setText(const std::wstring& text) override;

	virtual std::wstring getText() const override;

	virtual void setForeground() override;

	virtual bool isForeground() const override;

	virtual void setVisible(bool visible) override;

	virtual bool isVisible() const override;

	virtual void setEnable(bool enable) override;

	virtual bool isEnable() const override;

	virtual bool hasFocus() const override;

	virtual void setFocus() override;

	virtual bool hasCapture() const override;

	virtual void setCapture() override;

	virtual void releaseCapture() override;

	virtual void startTimer(int interval) override;

	virtual void stopTimer() override;

	virtual void setRect(const Rect& rect) override;

	virtual Rect getRect() const override;

	virtual Rect getInnerRect() const override;

	virtual Rect getNormalRect() const override;

	virtual void setFont(const Font& font) override;

	virtual Font getFont() const override;

	virtual const IFontMetric* getFontMetric() const override;

	virtual void setCursor(Cursor cursor) override;

	virtual Point getMousePosition(bool relative) const override;

	virtual Point screenToClient(const Point& pt) const override;

	virtual Point clientToScreen(const Point& pt) const override;

	virtual bool hitTest(const Point& pt) const override;

	virtual void setChildRects(const IWidgetRect* childRects, uint32_t count, bool redraw) override;

	virtual Size getMinimumSize() const override;

	virtual Size getPreferredSize(const Size& hint) const override;

	virtual Size getMaximumSize() const override;

	virtual void update(const Rect* rc, bool immediate) override;

	virtual int32_t dpi96(int32_t measure) const override;

	virtual int32_t invdpi96(int32_t measure) const override;

	virtual void* getInternalHandle() override;

	virtual SystemWindow getSystemWindow() override;

	virtual ITopLevelWidgetHost* getWidgetHost() override;

	virtual VirtualWidget* getVirtualWidget() override;

	// IFontMetric

	virtual void getAscentAndDescent(int32_t& outAscent, int32_t& outDescent) const override;

	virtual int32_t getAdvance(wchar_t ch, wchar_t next) const override;

	virtual int32_t getLineSpacing() const override;

	virtual Size getExtent(const std::wstring& text) const override;

	// TopLevelWidgetHost access

	EventSubject* getOwner() const { return m_owner; }

	VirtualWidget* getParentVirtual() const { return m_parent; }

	AlignedVector< VirtualWidget* >& getChildren() { return m_children; }

	uint32_t getStyle() const { return m_style; }

	Cursor getCursor() const { return m_cursor; }

	/*! Widget local coordinates to/from top-level client coordinates. */
	Point toHost(const Point& pt) const;

	Point fromHost(const Point& pt) const;

	Rect getHostRect() const;

	bool isEffectivelyVisible() const;

	bool isEffectivelyEnabled() const;

private:
	EventSubject* m_owner;
	Ref< TopLevelWidgetHost > m_host;
	VirtualWidget* m_parent = nullptr;
	AlignedVector< VirtualWidget* > m_children;
	Rect m_rect;
	Font m_font;
	Cursor m_cursor = Cursor::Arrow;
	std::wstring m_text;
	uint32_t m_style = 0;
	int32_t m_timer = -1;
	int32_t m_timerInterval = 0;
	bool m_visible = true;
	bool m_enable = true;

	const IFontMetricProvider* getProvider() const;

	void migrateHost(TopLevelWidgetHost* host);
};

}
