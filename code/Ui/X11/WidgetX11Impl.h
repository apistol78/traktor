/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include <X11/cursorfont.h> 
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cairo.h>
#include <cairo-xlib.h>
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/EventSubject.h"
#include "Ui/Events/AllEvents.h"
#include "Ui/Itf/IFontMetric.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/X11/Context.h"
#include "Ui/X11/CanvasX11.h"
#include "Ui/X11/Timers.h"
#include "Ui/X11/TypesX11.h"
#include "Ui/X11/UtilitiesX11.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

template < typename ControlType >
class WidgetX11Impl
:	public ControlType
,	public IFontMetric
{
public:
	explicit WidgetX11Impl(Context* context, EventSubject* owner)
	:	m_context(context)
	,	m_owner(owner)
	{
	}

	virtual ~WidgetX11Impl()
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

			if (m_xic != 0)
			{
				XDestroyIC(m_xic);
				m_xic = 0;
			}

			XDestroyWindow(m_context->getDisplay(), m_data.window);

			m_context = nullptr;

			m_data.window = 0;
			m_data.parent = nullptr;

			delete this;
		}
	}

	virtual void setParent(IWidget* parent) override
	{
		WidgetData* parentData = static_cast< WidgetData* >(parent->getInternalHandle());
		XReparentWindow(m_context->getDisplay(), m_data.window, parentData->window, 0, 0);
		m_data.parent = parentData;
	}

	virtual void setText(const std::wstring& text) override
	{
		m_text = text;
	}

	virtual std::wstring getText() const override
	{
		return m_text;
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
		if (visible != m_data.visible)
		{
			m_data.visible = visible;

			if (visible)	// Becoming visible.
			{
				if (m_rect.area() > 0)
				{
					if (!m_data.mapped)
					{
						XMapWindow(m_context->getDisplay(), m_data.window);
						m_data.mapped = true;
					}

					// Resize window.
					XMoveResizeWindow(m_context->getDisplay(), m_data.window, m_rect.left, m_rect.top, m_rect.getWidth(), m_rect.getHeight());
				}
				else
				{
					if (m_data.mapped)
					{
						XUnmapWindow(m_context->getDisplay(), m_data.window);
						m_data.mapped = false;
					}
				}
			}
			else	// Becoming hidden.
			{
				if (m_data.mapped)
				{
					XUnmapWindow(m_context->getDisplay(), m_data.window);
					m_data.mapped = false;
				}
			}

			ShowEvent showEvent(m_owner, visible);
			m_owner->raiseEvent(&showEvent);
		}
	}

	virtual bool isVisible() const override
	{
		return m_data.visible;
	}

	virtual void setEnable(bool enable) override
	{
		m_data.enable = enable;
	}

	virtual bool isEnable() const override
	{
		return m_data.enable;
	}

	virtual bool hasFocus() const override
	{
		return m_data.focus;
	}

	virtual void setFocus() override
	{
		m_context->setFocus(&m_data);
	}

	virtual bool hasCapture() const override
	{
		return m_data.grabbed;
	}

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
		m_timer = Timers::getInstance().bind(interval, [=](){
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

	virtual void setRect(const Rect& rect) override
	{
		if (m_rect == rect)
			return;

		const Size fromSize = m_rect.getSize();
		m_rect = rect;

		if (!m_data.visible)
			return;

		if (m_rect.area() > 0)
		{
			if (!m_data.mapped)
			{
				XMapWindow(m_context->getDisplay(), m_data.window);
				m_data.mapped = true;
			}

			// Resize window.
			XMoveResizeWindow(m_context->getDisplay(), m_data.window, m_rect.left, m_rect.top, m_rect.getWidth(), m_rect.getHeight());
		}
		else
		{
			if (m_data.mapped)
			{
				XUnmapWindow(m_context->getDisplay(), m_data.window);
				m_data.mapped = false;
			}
		}

		// Issue resized event if size changed, not if only position changed.
		if (m_rect.getSize() != fromSize)
		{
			SizeEvent sizeEvent(m_owner, m_rect.getSize());
			m_owner->raiseEvent(&sizeEvent);
		}
	}

	virtual Rect getRect() const override
	{
		return m_rect;
	}

	virtual Rect getInnerRect() const override
	{
		return Rect(0, 0, m_rect.getWidth(), m_rect.getHeight());
	}

	virtual Rect getNormalRect() const override
	{
		return Rect(0, 0, m_rect.getWidth(), m_rect.getHeight());
	}

	virtual void setFont(const Font& font) override
	{
		m_font = font;

		cairo_select_font_face(
			m_cairo,
			wstombs(m_font.getFace()).c_str(),
			CAIRO_FONT_SLANT_NORMAL,
			m_font.isBold() ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL
		);

		cairo_set_font_size(
			m_cairo,
			dpi96(m_font.getSize().get())
		);
	}

	virtual Font getFont() const override
	{
		return m_font;
	}

	virtual const IFontMetric* getFontMetric() const override
	{
		return this;
	}

	virtual void setCursor(Cursor cursor) override
	{
		int shape = -1;
		switch (cursor)
		{
		default:
		// case Cursor::None:
		case Cursor::Arrow:
			break;
		case Cursor::ArrowRight:
			break;
		case Cursor::ArrowWait:
			break;
		case Cursor::Cross:
			break;
		case Cursor::Hand:
			break;
		case Cursor::IBeam:
			break;
		case Cursor::SizeNESW:
			break;
		case Cursor::SizeNS:
			shape = XC_sb_v_double_arrow;
			break;
		case Cursor::SizeNWSE:
			break;
		case Cursor::SizeWE:
			shape = XC_sb_h_double_arrow;
			break;
		case Cursor::Sizing:
			break;
		case Cursor::Wait:
			break;
		}

		if (shape == m_currentShape)
			return;

		if (shape >= 0)
		{
			XID xc = XCreateFontCursor(m_context->getDisplay(), shape);
			XDefineCursor(m_context->getDisplay(), m_data.window, xc);
		}
		else
			XUndefineCursor(m_context->getDisplay(), m_data.window);

		m_currentShape = shape;
	}

	virtual Point getMousePosition(bool relative) const override
	{
		Window root, child;
		int rootX, rootY;
		int winX, winY;
		unsigned int mask;

		XQueryPointer(
			m_context->getDisplay(),
			relative ? m_data.window : DefaultRootWindow(m_context->getDisplay()),
			&root,
			&child,
			&rootX, &rootY,
			&winX, &winY,
			&mask
		);

		return Point(winX, winY);
	}

	virtual Point screenToClient(const Point& pt) const override
	{
		Window dw; int x, y;
		XTranslateCoordinates(m_context->getDisplay(), DefaultRootWindow(m_context->getDisplay()), m_data.window, pt.x, pt.y, &x, &y, &dw);
		return Point(x, y);
	}

	virtual Point clientToScreen(const Point& pt) const override
	{
		Window dw; int x, y;
		XTranslateCoordinates(m_context->getDisplay(), m_data.window, DefaultRootWindow(m_context->getDisplay()), pt.x, pt.y, &x, &y, &dw);
		return Point(x, y);
	}

	virtual bool hitTest(const Point& pt) const override
	{
		Point cpt = screenToClient(pt);
		return getInnerRect().inside(cpt);
	}

	virtual void setChildRects(const IWidgetRect* childRects, uint32_t count, bool redraw) override
	{
		for (uint32_t i = 0; i < count; ++i)
		{
			if (childRects[i].widget)
				childRects[i].widget->setRect(childRects[i].rect);
		}
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
		if (!m_data.visible || !m_data.mapped)
			return;

		if (!immediate)
		{
			if (m_pendingExposure)
				return;

			m_pendingExposure = true;

			XEvent xe = { 0 };
        	xe.type = Expose;
        	xe.xexpose.window = m_data.window;
        	XSendEvent(m_context->getDisplay(), m_data.window, False, ExposureMask, &xe);
		}
		else
		{
			draw(rc);
		}
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

	virtual void* getInternalHandle() override
	{
		return (void*)&m_data;
	}

	virtual SystemWindow getSystemWindow() override
	{
		return SystemWindow(m_context->getDisplay(), m_data.window);
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

protected:
	enum
	{
		_NET_WM_STATE_REMOVE = 0,
		_NET_WM_STATE_ADD = 1,
		_NET_WM_STATE_TOGGLE = 2
	};

	Ref< Context > m_context;
	EventSubject* m_owner = nullptr;

	WidgetData m_data;
	XIC m_xic = 0;

	Rect m_rect;
	Font m_font;
	int32_t m_currentShape = -1;

	cairo_surface_t* m_surface = nullptr;
	cairo_t* m_cairo = nullptr;

	std::wstring m_text;
	int32_t m_timer = -1;

	int32_t m_lastMousePress = 0;
	int32_t m_lastMouseButton = 0;
	bool m_pendingExposure = false;

	bool create(IWidget* parent, int32_t style, Window window, const Rect& rect, bool visible, bool topLevel)
	{
		if (window == 0)
			return false;

		m_data.window = window;
		m_data.parent = (parent != nullptr ? static_cast< WidgetData* >(parent->getInternalHandle()) : nullptr);
		m_data.topLevel = topLevel;
		m_data.visible = visible;
		m_data.mapped = false;

		m_rect = rect;

		if (topLevel)
		{
			XSelectInput(
				m_context->getDisplay(),
				m_data.window,
				ButtonPressMask |
				ButtonReleaseMask |
				StructureNotifyMask |
				KeyPressMask |
				KeyReleaseMask |
				ExposureMask |
				FocusChangeMask |
				PointerMotionMask |
				EnterWindowMask |
				LeaveWindowMask
			);
		}
		else
		{
			XSelectInput(
				m_context->getDisplay(),
				m_data.window,
				ButtonPressMask |
				ButtonReleaseMask |
				KeyPressMask |
				KeyReleaseMask |
				ExposureMask |
				FocusChangeMask |
				PointerMotionMask |
				EnterWindowMask |
				LeaveWindowMask
			);
		}

		if (visible && m_rect.area() > 0)
		{
	    	XMapWindow(m_context->getDisplay(), m_data.window);
			XFlush(m_context->getDisplay());
			m_data.mapped = true;
		}

		// Create input context.
		if ((m_xic = XCreateIC(
			m_context->getXIM(),
			XNInputStyle,   XIMPreeditNothing | XIMStatusNothing,
			XNClientWindow, m_data.window,
			XNFocusWindow,  m_data.window,
			nullptr
		)) == 0)
		{
			return false;
		}

		if ((style & WsNoCanvas) == 0)
		{
			m_surface = cairo_xlib_surface_create(
				m_context->getDisplay(),
				m_data.window,
				DefaultVisual(m_context->getDisplay(), m_context->getScreen()),
				m_rect.getWidth(),
				m_rect.getHeight()
			);

			m_cairo = cairo_create(m_surface);
			setFont(Font(L"Ubuntu Regular", 11_ut));
		}

		// Focus in.
		m_context->bind(&m_data, FocusIn, [this](XEvent& xe) {
			if (m_xic != 0)
				XSetICFocus(m_xic);
			FocusEvent focusEvent(m_owner, true);
			m_owner->raiseEvent(&focusEvent);
		});

		// Focus out.
		m_context->bind(&m_data, FocusOut, [this](XEvent& xe) {
			if (m_xic != 0)
				XUnsetICFocus(m_xic);
			FocusEvent focusEvent(m_owner, false);
			m_owner->raiseEvent(&focusEvent);
		});

		// Key press.
		m_context->bind(&m_data, KeyPress, [this](XEvent& xe) {
			T_FATAL_ASSERT (m_data.enable);

			int nkeysyms;
			KeySym* ks = XGetKeyboardMapping(m_context->getDisplay(), xe.xkey.keycode, 1, &nkeysyms);
			if (ks == nullptr)
				return;

			VirtualKey vk = translateToVirtualKey(ks, nkeysyms);
			if (vk != VkNull)
			{
				KeyDownEvent keyDownEvent(m_owner, vk, xe.xkey.keycode, 0);
				m_owner->raiseEvent(&keyDownEvent);
			}

			// Ensure owner is still valid; widget might have been destroyed in key down event.
			if (m_owner)
			{
				uint8_t str[8] = { 0 };

				Status status = 0;
				const int n = Xutf8LookupString(m_xic, &xe.xkey, (char*)str, 8, ks, &status);
				if (n > 0)
				{
					wchar_t wch = 0;
					if (Utf8Encoding().translate(str, n, wch) > 0)
					{
						KeyEvent keyEvent(m_owner, vk, xe.xkey.keycode, wch);
						m_owner->raiseEvent(&keyEvent);
					}
				}
			}

			XFree(ks);
		});

		// Key release.
		m_context->bind(&m_data, KeyRelease, [this](XEvent& xe) {
			T_FATAL_ASSERT (m_data.enable);

			int nkeysyms;
			KeySym* ks = XGetKeyboardMapping(m_context->getDisplay(), xe.xkey.keycode, 1, &nkeysyms);
			if (ks == nullptr)
				return;

			VirtualKey vk = translateToVirtualKey(ks, nkeysyms);
			if (vk != VkNull)
			{
				bool repeat = false;
				if (XPending(m_context->getDisplay()))
				{
					XEvent nextEvent;
					XPeekEvent(m_context->getDisplay(), &nextEvent);
					repeat = (nextEvent.type == KeyPress && nextEvent.xkey.time == xe.xkey.time && nextEvent.xkey.keycode == xe.xkey.keycode);
				}

				KeyUpEvent keyUpEvent(m_owner, vk, xe.xkey.keycode, 0, repeat);
				m_owner->raiseEvent(&keyUpEvent);
			}

			XFree(ks);
		});

		// Motion
		m_context->bind(&m_data, MotionNotify, [this](XEvent& xe){
			T_FATAL_ASSERT (m_data.enable);

			int32_t button = 0;
			if ((xe.xmotion.state & Button1Mask) != 0)
				button = MbtLeft;
			if ((xe.xmotion.state & Button2Mask) != 0)
				button = MbtMiddle;
			if ((xe.xmotion.state & Button3Mask) != 0)
				button = MbtRight;

			MouseMoveEvent mouseMoveEvent(
				m_owner,
				button,
				Point(xe.xmotion.x, xe.xmotion.y)
			);
			m_owner->raiseEvent(&mouseMoveEvent);
		});

		// Enter
		m_context->bind(&m_data, EnterNotify, [this](XEvent& xe){
			MouseTrackEvent mouseTrackEvent(m_owner, true);
			m_owner->raiseEvent(&mouseTrackEvent);
		});

		// Leave
		m_context->bind(&m_data, LeaveNotify, [this](XEvent& xe){
			MouseTrackEvent mouseTrackEvent(m_owner, false);
			m_owner->raiseEvent(&mouseTrackEvent);
		});

		// Button press.
		m_context->bind(&m_data, ButtonPress, [this](XEvent& xe){
			T_FATAL_ASSERT (m_data.enable);

			if (xe.xbutton.button == 4 || xe.xbutton.button == 5)
			{
				MouseWheelEvent mouseWheelEvent(
					m_owner,
					xe.xbutton.button == 4 ? 1 : -1,
					clientToScreen(Point(xe.xbutton.x, xe.xbutton.y))
				);
				m_owner->raiseEvent(&mouseWheelEvent);
			}
			else
			{
				int32_t button = 0;
				switch (xe.xbutton.button)
				{
				case Button1:
					button = MbtLeft;
					break;

				case Button2:
					button = MbtMiddle;
					break;

				case Button3:
					button = MbtRight;
					break;

				default:
					return;
				}

				setFocus();

				MouseButtonDownEvent mouseButtonDownEvent(
					m_owner,
					button,
					Point(xe.xbutton.x, xe.xbutton.y)
				);
				m_owner->raiseEvent(&mouseButtonDownEvent);

				int32_t dbt = xe.xbutton.time - m_lastMousePress;
				if (dbt <= 200 && m_lastMouseButton == button)
				{
					MouseDoubleClickEvent mouseDoubleClickEvent(
						m_owner,
						button,
						Point(xe.xbutton.x, xe.xbutton.y)
					);
					m_owner->raiseEvent(&mouseDoubleClickEvent);
				}

				m_lastMousePress = xe.xbutton.time;
				m_lastMouseButton = button;
			}
		});

		// Button release.
		m_context->bind(&m_data, ButtonRelease, [this](XEvent& xe){
			T_FATAL_ASSERT (m_data.enable);

			int32_t button = 0;
			switch (xe.xbutton.button)
			{
			case Button1:
				button = MbtLeft;
				break;

			case Button2:
				button = MbtMiddle;
				break;

			case Button3:
				button = MbtRight;
				break;

			default:
				return;
			}

			MouseButtonUpEvent mouseButtonUpEvent(
				m_owner,
				button,
				Point(xe.xbutton.x, xe.xbutton.y)
			);
			m_owner->raiseEvent(&mouseButtonUpEvent);
		});

		// Configure
		if (topLevel)
		{
			m_context->bind(&m_data, ConfigureNotify, [this](XEvent& xe){
				const Rect rect(
					Point(xe.xconfigure.x, xe.xconfigure.y),
					Size(xe.xconfigure.width, xe.xconfigure.height)
				);
				if (rect.getSize() != m_rect.getSize())
				{
					m_rect = rect;
					SizeEvent sizeEvent(m_owner, m_rect.getSize());
					m_owner->raiseEvent(&sizeEvent);
				}
			});
		}

		// Expose
		m_context->bind(&m_data, Expose, [this](XEvent& xe){
			if (xe.xexpose.count != 0)
				return;
			draw(nullptr);
		});

		return true;
	}

	void draw(const Rect* rc)
	{
		m_pendingExposure = false;

		if (!m_data.visible || !m_data.mapped)
			return;

		const Size sz = m_rect.getSize();
		if (sz.cx <= 0 || sz.cy <= 0)
			return;

		if (m_cairo != nullptr)
		{
			const int32_t cw = cairo_xlib_surface_get_width(m_surface);
			const int32_t ch = cairo_xlib_surface_get_height(m_surface);
			if (cw != sz.cx || ch != sz.cy)
				cairo_xlib_surface_set_size(m_surface, sz.cx, sz.cy);

			cairo_push_group_with_content(m_cairo, CAIRO_CONTENT_COLOR);

			CanvasX11 canvasImpl(m_cairo, m_context->getSystemDPI());
			Canvas canvas(&canvasImpl, reinterpret_cast< Widget* >(m_owner));
			PaintEvent paintEvent(
				m_owner,
				canvas,
				rc != nullptr ? *rc : Rect(Point(0, 0), sz)
			);
			m_owner->raiseEvent(&paintEvent);

			cairo_pop_group_to_source(m_cairo);
			cairo_paint(m_cairo);
			cairo_surface_flush(m_surface);
		}
		else
		{
			Canvas canvas(nullptr, reinterpret_cast< Widget* >(m_owner));
			PaintEvent p(
				m_owner,
				canvas,
				rc != nullptr ? *rc : Rect(Point(0, 0), sz)
			);
			m_owner->raiseEvent(&p);			
		}
	}

	void setWmProperty(const char* const property, int32_t value)
	{
		XEvent evt = { 0 };

		Atom atomWmState = XInternAtom(m_context->getDisplay(), "_NET_WM_STATE", False);
		Atom atomWmProperty = XInternAtom(m_context->getDisplay(), property, False);

		evt.type = ClientMessage;
		evt.xclient.window = m_data.window;
		evt.xclient.message_type = atomWmState;
		evt.xclient.format = 32;
		evt.xclient.data.l[0] = value;
		evt.xclient.data.l[1] = atomWmProperty;
		evt.xclient.data.l[2] = 0;
		evt.xclient.data.l[3] = 0;
		evt.xclient.data.l[4] = 0;

		XSendEvent(
			m_context->getDisplay(),
			RootWindow(m_context->getDisplay(), m_context->getScreen()),
			False,
			SubstructureRedirectMask | SubstructureNotifyMask,
			&evt
		);
	}

	void setWmProperty(const char* const property1, const char* const property2, int32_t value)
	{
		XEvent evt = { 0 };

		Atom atomWmState = XInternAtom(m_context->getDisplay(), "_NET_WM_STATE", False);
		Atom atomWmProperty1 = XInternAtom(m_context->getDisplay(), property1, False);
		Atom atomWmProperty2 = XInternAtom(m_context->getDisplay(), property2, False);

		evt.type = ClientMessage;
		evt.xclient.window = m_data.window;
		evt.xclient.message_type = atomWmState;
		evt.xclient.format = 32;
		evt.xclient.data.l[0] = value;
		evt.xclient.data.l[1] = atomWmProperty1;
		evt.xclient.data.l[2] = atomWmProperty2;
		evt.xclient.data.l[3] = 0;
		evt.xclient.data.l[4] = 0;

		XSendEvent(
			m_context->getDisplay(),
			RootWindow(m_context->getDisplay(), m_context->getScreen()),
			False,
			SubstructureRedirectMask | SubstructureNotifyMask,
			&evt
		);
	}
};

	}
}
