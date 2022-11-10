/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#import <Cocoa/Cocoa.h>
#import "Ui/Cocoa/NSWindowDelegateProxy.h"

#include "Ui/Itf/IFontMetric.h"
#include "Ui/Itf/IToolForm.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

class ToolFormCocoa
:	public IToolForm
,	public IFontMetric
,	public INSWindowEventsCallback
{
public:
	ToolFormCocoa(EventSubject* owner);

	// IToolForm implementation

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style) override final;

	virtual void setIcon(ISystemBitmap* icon) override final;

	virtual DialogResult showModal() override final;

	virtual void endModal(DialogResult result) override final;

	// IWidget implementation

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

	virtual void setChildRects(const IWidgetRect* childRects, uint32_t count) override;

	virtual Size getMinimumSize() const override;

	virtual Size getPreferredSize(const Size& hint) const override;

	virtual Size getMaximumSize() const override;

	virtual void update(const Rect* rc, bool immediate) override;

	virtual void* getInternalHandle() override;

	virtual SystemWindow getSystemWindow() override;

	// IFontMetric

	virtual void getAscentAndDescent(int32_t& outAscent, int32_t& outDescent) const override final;

	virtual int32_t getAdvance(wchar_t ch, wchar_t next) const override final;

	virtual int32_t getLineSpacing() const override final;

	virtual Size getExtent(const std::wstring& text) const override final;

	// INSWindowEventsCallback

	virtual void event_windowDidMove() override;

	virtual void event_windowDidResize() override;

	virtual bool event_windowShouldClose() override;

	virtual void event_windowDidBecomeKey() override;

	virtual void event_windowDidResignKey() override;

	virtual void event_windowDidBecomeMain() override;

	virtual void event_windowDidResignMain() override;

private:
	EventSubject* m_owner = nullptr;
	IWidget* m_parent = nullptr;
	NSWindow* m_window = nullptr;
	NSTimer* m_timer = nullptr;
	DialogResult m_result = 0;
	
	void callbackTimer(id controlId);
};

	}
}

