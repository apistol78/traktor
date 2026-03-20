/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <cstring>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <xkbcommon/xkbcommon.h>
#include <cairo.h>
#include <linux/input-event-codes.h>
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/EventSubject.h"
#include "Ui/Events/AllEvents.h"
#include "Ui/Itf/IFontMetric.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/Wl/ContextWl.h"
#include "Ui/Wl/CanvasWl.h"
#include "Ui/Wl/Timers.h"
#include "Ui/Wl/TypesWl.h"
#include "Ui/Wl/UtilitiesWl.h"

namespace traktor::ui
{

class EventSubject;

/*
 * All coordinates exposed through the IWidget interface (setRect, getRect,
 * getInnerRect, pointer events, SizeEvent, etc.) are in **device pixels**,
 * matching the X11 backend.  Conversion to/from Wayland logical coordinates
 * happens exclusively inside this template at the Wayland API boundary.
 */

template < typename ControlType >
class WidgetWlImpl
:	public ControlType
,	public IFontMetric
{
public:
	explicit WidgetWlImpl(ContextWl* context, EventSubject* owner)
	:	m_context(context)
	,	m_owner(owner)
	{
	}

	virtual ~WidgetWlImpl()
	{
		T_FATAL_ASSERT(m_timer < 0);
		T_FATAL_ASSERT(m_surface == nullptr);
		T_FATAL_ASSERT(m_cairo == nullptr);
		T_FATAL_ASSERT(m_data.grabbed == false);
	}

	virtual void destroy() override
	{
		stopTimer();
		releaseCapture();

		if (m_context != nullptr)
		{
			m_context->unbind(&m_data);

			if (m_cairo != nullptr)
			{
				cairo_destroy(m_cairo);
				m_cairo = nullptr;
			}

			if (m_surface != nullptr)
			{
				cairo_surface_destroy(m_surface);
				m_surface = nullptr;
			}

			if (m_shmData != nullptr && m_shmSize > 0)
			{
				munmap(m_shmData, m_shmSize);
				m_shmData = nullptr;
				m_shmSize = 0;
			}

			if (m_buffer != nullptr)
			{
				wl_buffer_destroy(m_buffer);
				m_buffer = nullptr;
			}

			if (m_data.decoration != nullptr)
			{
				zxdg_toplevel_decoration_v1_destroy(m_data.decoration);
				m_data.decoration = nullptr;
			}

			if (m_data.xdgToplevel != nullptr)
			{
				xdg_toplevel_destroy(m_data.xdgToplevel);
				m_data.xdgToplevel = nullptr;
			}

			if (m_data.xdgPopup != nullptr)
			{
				xdg_popup_destroy(m_data.xdgPopup);
				m_data.xdgPopup = nullptr;
			}

			if (m_data.xdgSurface != nullptr)
			{
				xdg_surface_destroy(m_data.xdgSurface);
				m_data.xdgSurface = nullptr;
			}

			if (m_data.subsurface != nullptr)
			{
				wl_subsurface_destroy(m_data.subsurface);
				m_data.subsurface = nullptr;
			}

			if (m_data.surface != nullptr)
			{
				wl_surface_destroy(m_data.surface);
				m_data.surface = nullptr;
			}

			m_context = nullptr;
			m_data.parent = nullptr;

			delete this;
		}
	}

	virtual void setParent(IWidget* parent) override
	{
		WidgetData* parentData = static_cast< WidgetData* >(parent->getInternalHandle());

		if (m_data.subsurface != nullptr)
		{
			wl_subsurface_destroy(m_data.subsurface);
			m_data.subsurface = nullptr;
		}

		if (parentData->surface != nullptr)
		{
			m_data.subsurface = wl_subcompositor_get_subsurface(
				m_context->getSubcompositor(),
				m_data.surface,
				parentData->surface
			);
			wl_subsurface_set_desync(m_data.subsurface);
		}

		m_data.parent = parentData;
	}

	virtual void setText(const std::wstring& text) override { m_text = text; }
	virtual std::wstring getText() const override { return m_text; }
	virtual void setForeground() override { }
	virtual bool isForeground() const override { return false; }

	virtual void setVisible(bool visible) override
	{
		if (visible != m_data.visible)
		{
			m_data.visible = visible;

			if (visible)
			{
				if (m_rect.area() > 0)
				{
					if (!m_data.mapped)
					{
						if (m_data.topLevel && m_data.xdgToplevel)
						{
							m_data.mapped = true;
						}
						else if (m_data.subsurface)
						{
							wl_subsurface_set_position(m_data.subsurface, toLogical(m_rect.left), toLogical(m_rect.top));
							m_data.mapped = true;
						}
					}
				}

				SizeEvent sizeEvent(m_owner, m_rect.getSize());
				m_owner->raiseEvent(&sizeEvent);

				if (m_data.mapped)
					draw(nullptr);
			}
			else
			{
				if (m_data.mapped)
				{
					wl_surface_attach(m_data.surface, nullptr, 0, 0);
					wl_surface_commit(m_data.surface);
					m_data.mapped = false;
				}
			}

			ShowEvent showEvent(m_owner, visible);
			m_owner->raiseEvent(&showEvent);
		}
	}

	virtual bool isVisible() const override { return m_data.visible; }
	virtual void setEnable(bool enable) override { m_data.enable = enable; }
	virtual bool isEnable() const override { return m_data.enable; }

	virtual bool hasFocus() const override
	{
		return m_context->getInternalFocus() == &m_data;
	}

	virtual void setFocus() override
	{
		m_context->setInternalFocus(&m_data);
	}

	virtual bool hasCapture() const override { return m_data.grabbed; }

	virtual void setCapture() override
	{
		if (!m_data.grabbed)
			m_context->grab(&m_data);
	}

	virtual void releaseCapture() override
	{
		if (m_data.grabbed)
			m_context->ungrab(&m_data);
	}

	virtual void startTimer(int interval) override
	{
		stopTimer();
		m_timer = Timers::getInstance().bind(interval, [=, this](){
			if (!isVisible())
				return;
			TimerEvent timerEvent(m_owner);
			m_owner->raiseEvent(&timerEvent);
		});
	}

	virtual void stopTimer() override
	{
		if (m_timer >= 0)
		{
			Timers::getInstance().unbind(m_timer);
			m_timer = -1;
		}
	}

	// rect is in device coordinates.
	virtual void setRect(const Rect& rect) override
	{
		if (m_rect == rect)
			return;

		const Size fromSize = m_rect.getSize();
		const bool wasMapped = m_data.mapped;
		m_rect = rect;

		if (!m_data.visible)
			return;

		if (m_rect.area() > 0)
		{
			if (!m_data.mapped)
			{
				if (m_data.subsurface)
					wl_subsurface_set_position(m_data.subsurface, toLogical(m_rect.left), toLogical(m_rect.top));
				m_data.mapped = true;
			}
			else
			{
				if (m_data.subsurface)
					wl_subsurface_set_position(m_data.subsurface, toLogical(m_rect.left), toLogical(m_rect.top));
			}
		}
		else
		{
			if (m_data.mapped)
			{
				wl_surface_attach(m_data.surface, nullptr, 0, 0);
				wl_surface_commit(m_data.surface);
				m_data.mapped = false;
			}
		}

		if (m_rect.getSize() != fromSize)
		{
			SizeEvent sizeEvent(m_owner, m_rect.getSize());
			m_owner->raiseEvent(&sizeEvent);
		}

		if (m_data.mapped && m_rect.area() > 0 && !m_pendingExposure)
		{
			m_pendingExposure = true;
			m_context->queueExpose(&m_data);
		}
	}

	// All rects returned in device coordinates.
	virtual Rect getRect() const override { return m_rect; }
	virtual Rect getInnerRect() const override { return Rect(0, 0, m_rect.getWidth(), m_rect.getHeight()); }
	virtual Rect getNormalRect() const override { return Rect(0, 0, m_rect.getWidth(), m_rect.getHeight()); }

	virtual void setFont(const Font& font) override
	{
		m_font = font;
		if (m_cairo)
		{
			cairo_select_font_face(
				m_cairo,
				wstombs(m_font.getFace()).c_str(),
				CAIRO_FONT_SLANT_NORMAL,
				m_font.isBold() ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL
			);
			cairo_set_font_size(m_cairo, dpi96(m_font.getSize().get()));
		}
	}

	virtual Font getFont() const override { return m_font; }
	virtual const IFontMetric* getFontMetric() const override { return this; }

	virtual void setCursor(Cursor cursor) override
	{
		const char* cursorName = nullptr;
		switch (cursor)
		{
		default:
		case Cursor::Arrow:     cursorName = "left_ptr"; break;
		case Cursor::Hand:      cursorName = "hand2"; break;
		case Cursor::IBeam:     cursorName = "xterm"; break;
		case Cursor::SizeNS:    cursorName = "sb_v_double_arrow"; break;
		case Cursor::SizeWE:    cursorName = "sb_h_double_arrow"; break;
		case Cursor::SizeNESW:  cursorName = "bottom_left_corner"; break;
		case Cursor::SizeNWSE:  cursorName = "bottom_right_corner"; break;
		case Cursor::Cross:     cursorName = "crosshair"; break;
		case Cursor::Wait:      cursorName = "watch"; break;
		case Cursor::ArrowWait: cursorName = "left_ptr_watch"; break;
		case Cursor::Sizing:    cursorName = "fleur"; break;
		}

		if (!cursorName)
			return;

		struct wl_pointer* pointer = m_context->getPointer();
		struct wl_cursor_theme* theme = m_context->getCursorTheme();
		struct wl_surface* cursorSurface = m_context->getCursorSurface();
		if (!pointer || !theme || !cursorSurface)
			return;

		struct wl_cursor* wlCursor = wl_cursor_theme_get_cursor(theme, cursorName);
		if (!wlCursor || wlCursor->image_count < 1)
			return;

		struct wl_cursor_image* image = wlCursor->images[0];
		struct wl_buffer* buffer = wl_cursor_image_get_buffer(image);

		wl_surface_attach(cursorSurface, buffer, 0, 0);
		wl_surface_damage(cursorSurface, 0, 0, image->width, image->height);
		wl_surface_commit(cursorSurface);

		wl_pointer_set_cursor(
			pointer,
			m_context->getPointerSerial(),
			cursorSurface,
			image->hotspot_x,
			image->hotspot_y
		);
	}

	// Returns device coordinates.
	virtual Point getMousePosition(bool relative) const override
	{
		if (relative)
			return m_lastPointerPos;
		else
			return clientToScreen(m_lastPointerPos);
	}

	virtual Point screenToClient(const Point& pt) const override
	{
		if (m_data.topLevel)
			return pt;
		return Point(pt.x - m_rect.left, pt.y - m_rect.top);
	}

	virtual Point clientToScreen(const Point& pt) const override
	{
		if (m_data.topLevel)
			return pt;
		return Point(pt.x + m_rect.left, pt.y + m_rect.top);
	}

	virtual bool hitTest(const Point& pt) const override
	{
		return getInnerRect().inside(screenToClient(pt));
	}

	virtual void setChildRects(const IWidgetRect* childRects, uint32_t count, bool redraw) override
	{
		for (uint32_t i = 0; i < count; ++i)
		{
			if (childRects[i].widget)
				childRects[i].widget->setRect(childRects[i].rect);
		}
	}

	virtual Size getMinimumSize() const override { return Size(0, 0); }
	virtual Size getPreferredSize(const Size& hint) const override { return Size(dpi96(128), dpi96(64)); }
	virtual Size getMaximumSize() const override { return Size(65535, 65535); }

	virtual void update(const Rect* rc, bool immediate) override
	{
		if (!m_data.visible || !m_data.mapped)
			return;

		if (!immediate)
		{
			if (m_pendingExposure)
				return;
			m_pendingExposure = true;
			m_context->queueExpose(&m_data);
		}
		else
			draw(rc);
	}

	virtual int32_t dpi96(int32_t measure) const override
	{
		const int32_t dpiw = m_context->getSystemDPI();
		return (dpiw * measure) / 96;
	}

	virtual int32_t invdpi96(int32_t measure) const override
	{
		const int32_t dpiw = m_context->getSystemDPI();
		return (96 * measure) / (dpiw > 0 ? dpiw : 96);
	}

	virtual void* getInternalHandle() override { return (void*)&m_data; }

	virtual SystemWindow getSystemWindow() override
	{
		return SystemWindow(
			m_context->getDisplay(),
			(unsigned long)(uintptr_t)m_data.surface,
			m_rect.getWidth(),
			m_rect.getHeight()
		);
	}

	// IFontMetric

	virtual void getAscentAndDescent(int32_t& outAscent, int32_t& outDescent) const override
	{
		T_FATAL_ASSERT(m_surface != nullptr);
		cairo_font_extents_t x;
		cairo_font_extents(m_cairo, &x);
		outAscent = (int32_t)x.ascent;
		outDescent = (int32_t)x.descent;
	}

	virtual int32_t getAdvance(wchar_t ch, wchar_t next) const override
	{
		T_FATAL_ASSERT(m_surface != nullptr);
		uint8_t uc[IEncoding::MaxEncodingSize + 1] = { 0 };
		int32_t nuc = Utf8Encoding().translate(&ch, 1, uc);
		if (nuc <= 0)
			return 0;
		cairo_text_extents_t tx;
		cairo_text_extents(m_cairo, (const char*)uc, &tx);
		return (int32_t)tx.x_advance;
	}

	virtual int32_t getLineSpacing() const override
	{
		T_FATAL_ASSERT(m_surface != nullptr);
		cairo_font_extents_t x;
		cairo_font_extents(m_cairo, &x);
		return (int32_t)x.height;
	}

	virtual Size getExtent(const std::wstring& text) const override
	{
		T_FATAL_ASSERT(m_surface != nullptr);
		cairo_font_extents_t fx;
		cairo_text_extents_t tx;
		cairo_font_extents(m_cairo, &fx);
		cairo_text_extents(m_cairo, wstombs(text).c_str(), &tx);
		return Size(tx.width, fx.height);
	}

	ContextWl* getContextWl() const { return m_context; }

protected:
	Ref< ContextWl > m_context;
	EventSubject* m_owner = nullptr;
	WidgetData m_data;
	Rect m_rect;			// Device coordinates.
	Font m_font;
	cairo_surface_t* m_surface = nullptr;
	cairo_t* m_cairo = nullptr;
	struct wl_buffer* m_buffer = nullptr;
	void* m_shmData = nullptr;
	int32_t m_shmSize = 0;
	std::wstring m_text;
	int32_t m_timer = -1;
	int32_t m_lastMouseButton = 0;
	uint32_t m_lastMousePress = 0;
	Point m_lastPointerPos;		// Device coordinates.
	bool m_pendingExposure = false;

	// --- Coordinate helpers ---

	int32_t toLogical(int32_t device) const
	{
		const int32_t s = m_context->getOutputScale();
		return (s > 1) ? device / s : device;
	}

	int32_t toDevice(int32_t logical) const
	{
		const int32_t s = m_context->getOutputScale();
		return logical * s;
	}

	// --- Creation ---

	bool create(IWidget* parent, int32_t style, bool visible, bool topLevel)
	{
		m_data.parent = (parent != nullptr ? static_cast< WidgetData* >(parent->getInternalHandle()) : nullptr);
		m_data.topLevel = topLevel;
		m_data.visible = visible;
		m_data.mapped = false;

		m_data.surface = wl_compositor_create_surface(m_context->getCompositor());
		if (!m_data.surface)
			return false;

		if (!topLevel && m_data.parent && m_data.parent->surface)
		{
			m_data.subsurface = wl_subcompositor_get_subsurface(
				m_context->getSubcompositor(),
				m_data.surface,
				m_data.parent->surface
			);
			if (m_data.subsurface)
			{
				wl_subsurface_set_desync(m_data.subsurface);
				wl_subsurface_set_position(m_data.subsurface, toLogical(m_rect.left), toLogical(m_rect.top));
			}
		}

		if ((style & WsNoCanvas) == 0)
		{
			ensureSurface();
			setFont(Font(L"Ubuntu Regular", 11_ut));
		}

		// Focus events
		m_context->bind(&m_data, WlEvtFocusIn, [=, this](WlEvent& e) {
			FocusEvent focusEvent(m_owner, true);
			m_owner->raiseEvent(&focusEvent);
		});

		m_context->bind(&m_data, WlEvtFocusOut, [=, this](WlEvent& e) {
			FocusEvent focusEvent(m_owner, false);
			m_owner->raiseEvent(&focusEvent);
		});

		// Key press/release — coordinates already in device via preTranslate/event queue.
		m_context->bind(&m_data, WlEvtKeyboardKey, [=, this](WlEvent& e) {
			T_FATAL_ASSERT(m_data.enable);

			struct xkb_state* xkbState = m_context->getXkbState();
			if (!xkbState)
				return;

			const uint32_t keycode = e.key + 8;
			const xkb_keysym_t sym = xkb_state_key_get_one_sym(xkbState, keycode);
			VirtualKey vk = translateToVirtualKey(sym);

			if (e.keyState == WL_KEYBOARD_KEY_STATE_PRESSED)
			{
				if (vk != VkNull)
				{
					KeyDownEvent keyDownEvent(m_owner, vk, e.key, 0);
					m_owner->raiseEvent(&keyDownEvent);
				}
				if (m_owner)
				{
					char buf[8] = { 0 };
					int n = xkb_state_key_get_utf8(xkbState, keycode, buf, sizeof(buf));
					if (n > 0)
					{
						wchar_t wch = 0;
						if (Utf8Encoding().translate((const uint8_t*)buf, n, wch) > 0)
						{
							KeyEvent keyEvent(m_owner, vk, e.key, wch);
							m_owner->raiseEvent(&keyEvent);
						}
					}
				}
			}
			else
			{
				if (vk != VkNull)
				{
					KeyUpEvent keyUpEvent(m_owner, vk, e.key, 0, false);
					m_owner->raiseEvent(&keyUpEvent);
				}
			}
		});

		// Pointer events — pointerX/Y are already in device coords (scaled in ContextWl).
		m_context->bind(&m_data, WlEvtPointerMotion, [=, this](WlEvent& e) {
			T_FATAL_ASSERT(m_data.enable);
			m_lastPointerPos = Point((int)e.pointerX, (int)e.pointerY);
			int32_t button = 0;
			MouseMoveEvent mouseMoveEvent(m_owner, button, m_lastPointerPos);
			m_owner->raiseEvent(&mouseMoveEvent);
		});

		m_context->bind(&m_data, WlEvtPointerEnter, [=, this](WlEvent& e) {
			m_lastPointerPos = Point((int)e.pointerX, (int)e.pointerY);
			MouseTrackEvent mouseTrackEvent(m_owner, true);
			m_owner->raiseEvent(&mouseTrackEvent);
		});

		m_context->bind(&m_data, WlEvtPointerLeave, [=, this](WlEvent& e) {
			MouseTrackEvent mouseTrackEvent(m_owner, false);
			m_owner->raiseEvent(&mouseTrackEvent);
		});

		m_context->bind(&m_data, WlEvtPointerButton, [=, this](WlEvent& e) {
			T_FATAL_ASSERT(m_data.enable);

			int32_t button = 0;
			switch (e.button)
			{
			case BTN_LEFT:   button = MbtLeft; break;
			case BTN_MIDDLE: button = MbtMiddle; break;
			case BTN_RIGHT:  button = MbtRight; break;
			default: return;
			}

			if (e.buttonState == WL_POINTER_BUTTON_STATE_PRESSED)
			{
				if ((style & WsFocus) != 0)
					setFocus();

				MouseButtonDownEvent mouseButtonDownEvent(m_owner, button, m_lastPointerPos);
				m_owner->raiseEvent(&mouseButtonDownEvent);

				const uint32_t now = e.serial;
				if ((int32_t)(now - m_lastMousePress) <= 200 && m_lastMouseButton == button)
				{
					MouseDoubleClickEvent mouseDoubleClickEvent(m_owner, button, m_lastPointerPos);
					m_owner->raiseEvent(&mouseDoubleClickEvent);
				}
				m_lastMousePress = now;
				m_lastMouseButton = button;
			}
			else
			{
				MouseButtonUpEvent mouseButtonUpEvent(m_owner, button, m_lastPointerPos);
				m_owner->raiseEvent(&mouseButtonUpEvent);
			}
		});

		m_context->bind(&m_data, WlEvtPointerAxis, [=, this](WlEvent& e) {
			if (e.axisType == WL_POINTER_AXIS_VERTICAL_SCROLL)
			{
				int32_t rotation = (e.axisValue < 0.0) ? 1 : -1;
				MouseWheelEvent mouseWheelEvent(m_owner, rotation, clientToScreen(m_lastPointerPos));
				m_owner->raiseEvent(&mouseWheelEvent);
			}
		});

		m_context->bind(&m_data, WlEvtExpose, [this](WlEvent& e) {
			draw(nullptr);
		});

		if (visible && m_rect.area() > 0)
		{
			wl_surface_commit(m_data.surface);
			m_data.mapped = true;
		}

		return true;
	}

	// --- Buffer / surface ---

	void ensureSurface()
	{
		// m_rect is in device pixels — buffer matches exactly.
		const int32_t w = std::max(m_rect.getWidth(), 1);
		const int32_t h = std::max(m_rect.getHeight(), 1);
		const int32_t stride = w * 4;
		const int32_t size = stride * h;

		if (m_surface != nullptr)
		{
			const int32_t cw = cairo_image_surface_get_width(m_surface);
			const int32_t ch = cairo_image_surface_get_height(m_surface);
			if (cw == w && ch == h)
				return;

			cairo_destroy(m_cairo);
			m_cairo = nullptr;
			cairo_surface_destroy(m_surface);
			m_surface = nullptr;
		}

		if (m_shmData != nullptr && m_shmSize > 0)
		{
			munmap(m_shmData, m_shmSize);
			m_shmData = nullptr;
			m_shmSize = 0;
		}

		if (m_buffer != nullptr)
		{
			wl_buffer_destroy(m_buffer);
			m_buffer = nullptr;
		}

		m_buffer = m_context->createShmBuffer(w, h, stride, &m_shmData);
		if (!m_buffer)
			return;

		m_shmSize = size;

		m_surface = cairo_image_surface_create_for_data(
			static_cast< unsigned char* >(m_shmData),
			CAIRO_FORMAT_ARGB32,
			w, h, stride
		);
		m_cairo = cairo_create(m_surface);

		// Buffer is in device pixels; tell compositor the scale so it
		// maps to the correct logical size on screen.
		const int32_t scale = m_context->getOutputScale();
		wl_surface_set_buffer_scale(m_data.surface, scale);

		// Re-apply font on the new cairo context.
		if (!m_font.getFace().empty())
		{
			cairo_select_font_face(
				m_cairo,
				wstombs(m_font.getFace()).c_str(),
				CAIRO_FONT_SLANT_NORMAL,
				m_font.isBold() ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL
			);
			cairo_set_font_size(m_cairo, dpi96(m_font.getSize().get()));
		}
	}

	// --- Drawing ---

	void draw(const Rect* rc)
	{
		m_pendingExposure = false;

		if (!m_data.visible || !m_data.mapped)
			return;

		const Size sz = m_rect.getSize();
		if (sz.cx <= 0 || sz.cy <= 0)
			return;

		ensureSurface();

		if (m_cairo != nullptr)
		{
			cairo_push_group_with_content(m_cairo, CAIRO_CONTENT_COLOR);

			CanvasWl canvasImpl(m_cairo, m_context->getSystemDPI());
			Canvas canvas(&canvasImpl, reinterpret_cast< Widget* >(m_owner));

			PaintEvent paintEvent(
				m_owner, canvas,
				rc != nullptr ? *rc : Rect(Point(0, 0), sz)
			);
			m_owner->raiseEvent(&paintEvent);

			OverlayPaintEvent overlayPaintEvent(
				m_owner, canvas,
				rc != nullptr ? *rc : Rect(Point(0, 0), sz)
			);
			m_owner->raiseEvent(&overlayPaintEvent);

			cairo_pop_group_to_source(m_cairo);
			cairo_paint(m_cairo);
			cairo_surface_flush(m_surface);

			// Damage in buffer coordinates (device pixels).
			wl_surface_attach(m_data.surface, m_buffer, 0, 0);
			wl_surface_damage_buffer(m_data.surface, 0, 0, sz.cx, sz.cy);
			wl_surface_commit(m_data.surface);
		}
		else
		{
			Canvas canvas(nullptr, reinterpret_cast< Widget* >(m_owner));
			PaintEvent p(
				m_owner, canvas,
				rc != nullptr ? *rc : Rect(Point(0, 0), sz)
			);
			m_owner->raiseEvent(&p);
		}
	}
};

}
