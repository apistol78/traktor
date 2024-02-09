/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Config.h"
#include "Core/Platform.h"
#include "Ui/Enums.h"
#include "Ui/Rect.h"
#include "Ui/Font.h"

namespace traktor::ui
{

class IFontMetric;

/*!
 * \ingroup UI
 */
struct IWidgetRect
{
	class IWidget* widget;
	Rect rect;
};

/*! Widget interface.
 * \ingroup UI
 */
class IWidget
{
public:
	virtual ~IWidget() {}

	virtual void destroy() = 0;

	virtual void setParent(IWidget* parent) = 0;

	virtual void setText(const std::wstring& text) = 0;

	virtual std::wstring getText() const = 0;

	virtual void setForeground() = 0;

	virtual bool isForeground() const = 0;

	virtual void setVisible(bool visible) = 0;

	virtual bool isVisible() const = 0;

	virtual void setEnable(bool enable) = 0;

	virtual bool isEnable() const = 0;

	virtual bool hasFocus() const = 0;

	virtual void setFocus() = 0;

	virtual bool hasCapture() const = 0;

	virtual void setCapture() = 0;

	virtual void releaseCapture() = 0;

	virtual void startTimer(int interval) = 0;

	virtual void stopTimer() = 0;

	virtual void setRect(const Rect& rect) = 0;

	virtual Rect getRect() const = 0;

	virtual Rect getInnerRect() const = 0;

	virtual Rect getNormalRect() const = 0;

	virtual void setFont(const Font& font) = 0;

	virtual Font getFont() const = 0;

	virtual const IFontMetric* getFontMetric() const = 0;

	virtual void setCursor(Cursor cursor) = 0;

	virtual Point getMousePosition(bool relative) const = 0;

	virtual Point screenToClient(const Point& pt) const = 0;

	virtual Point clientToScreen(const Point& pt) const = 0;

	virtual bool hitTest(const Point& pt) const = 0;

	virtual void setChildRects(const IWidgetRect* childRects, uint32_t count, bool redraw) = 0;

	virtual Size getMinimumSize() const = 0;

	virtual Size getPreferredSize(const Size& hint) const = 0;

	virtual Size getMaximumSize() const = 0;

	virtual void update(const Rect* rc, bool immediate) = 0;

	virtual int32_t dpi96(int32_t measure) const = 0;

	virtual int32_t invdpi96(int32_t measure) const = 0;

	virtual void* getInternalHandle() = 0;

	virtual SystemWindow getSystemWindow() = 0;
};

}

