/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/Itf/IFontMetric.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/WL/Context.h"

namespace traktor::ui
{

class EventSubject;

template < typename ControlType >
class WidgetWLImpl
:	public ControlType
,	public IFontMetric
{
public:
	explicit WidgetWLImpl(Context* context, EventSubject* owner)
	:	m_context(context)
	,	m_owner(owner)
	{
	}

	virtual ~WidgetWLImpl()
	{
	}

	virtual void destroy() override
	{
		delete this;
	}

	virtual void setParent(IWidget* parent) override
	{
	}

	virtual void setText(const std::wstring& text) override
	{
	}

	virtual std::wstring getText() const override
	{
		return L"";
	}

	virtual void setForeground() override
	{
	}

	virtual bool isForeground() const override
	{
		return false;
	}

	virtual void setVisible(bool visible) override
	{
	}

	virtual bool isVisible() const override
	{
		return true;
	}

	virtual void setEnable(bool enable) override
	{
	}

	virtual bool isEnable() const override
	{
		return true;
	}

	virtual bool hasFocus() const override
	{
		return false;
	}

	virtual void setFocus() override
	{
	}

	virtual bool hasCapture() const override
	{
		return false;
	}

	virtual void setCapture() override
	{
	}

	virtual void releaseCapture() override
	{
	}

	virtual void startTimer(int interval) override
	{
	}

	virtual void stopTimer() override
	{
	}

	virtual void setRect(const Rect& rect) override
	{
	}

	virtual Rect getRect() const override
	{
		return Rect();
	}

	virtual Rect getInnerRect() const override
	{
		return Rect();
	}

	virtual Rect getNormalRect() const override
	{
		return Rect();
	}

	virtual void setFont(const Font& font) override
	{
	}

	virtual Font getFont() const override
	{
		return Font();
	}

	virtual const IFontMetric* getFontMetric() const override
	{
		return this;
	}

	virtual void setCursor(Cursor cursor) override
	{
	}

	virtual Point getMousePosition(bool relative) const override
	{
		return Point(0, 0);
	}

	virtual Point screenToClient(const Point& pt) const override
	{
		return Point(0, 0);
	}

	virtual Point clientToScreen(const Point& pt) const override
	{
		return Point(0, 0);
	}

	virtual bool hitTest(const Point& pt) const override
	{
        return false;
    }

	virtual void setChildRects(const IWidgetRect* childRects, uint32_t count, bool redraw) override
	{
	}

	virtual Size getMinimumSize() const override
	{
		return Size(0, 0);
	}

	virtual Size getPreferredSize(const Size& hint) const override
	{
		return Size(128, 64);
	}

	virtual Size getMaximumSize() const override
	{
		return Size(65535, 65535);
	}

	virtual void update(const Rect* rc, bool immediate) override
	{
	}

	virtual int32_t dpi96(int32_t measure) const override
	{
		return measure;
	}

	virtual int32_t invdpi96(int32_t measure) const override
	{
		return measure;
	}

	virtual void* getInternalHandle() override
	{
		return nullptr;
	}

	virtual SystemWindow getSystemWindow() override
	{
		return SystemWindow();
	}

	// IFontMetric

	virtual void getAscentAndDescent(int32_t& outAscent, int32_t& outDescent) const override
	{
	}

	virtual int32_t getAdvance(wchar_t ch, wchar_t next) const override
	{
        return 0;
    }

	virtual int32_t getLineSpacing() const override
	{
        return 0;
	}

	virtual Size getExtent(const std::wstring& text) const override
	{
		return Size(0, 0);
	}

protected:
	Context* m_context = nullptr;
    EventSubject* m_owner = nullptr;
};

}
