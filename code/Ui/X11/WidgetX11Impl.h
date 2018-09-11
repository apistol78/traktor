#ifndef traktor_ui_WidgetX11Impl_H
#define traktor_ui_WidgetX11Impl_H

#include <map>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cairo.h>
#include <cairo-xlib.h>
#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Ui/Application.h"
#include "Ui/Canvas.h"
#include "Ui/EventSubject.h"
#include "Ui/Events/AllEvents.h"
#include "Ui/Itf/IFontMetric.h"
#include "Ui/Itf/IWidget.h"
#include "Ui/X11/Assoc.h"
#include "Ui/X11/CanvasX11.h"
#include "Ui/X11/Timers.h"
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
	WidgetX11Impl(EventSubject* owner, Display* display, int32_t screen)
	:	m_owner(owner)
	,	m_display(display)
	,	m_screen(screen)
	,	m_window(0)
	,	m_surface(nullptr)
	,	m_context(nullptr)
	,	m_visible(false)
	,	m_enable(true)
	,	m_grabbed(false)
	,	m_lastMousePress(0)
	,	m_lastMouseButton(0)
	,	m_pendingDraw(false)
	{
	}

	virtual ~WidgetX11Impl()
	{
		T_FATAL_ASSERT(m_timers.empty());
		T_FATAL_ASSERT(m_surface == nullptr);
		T_FATAL_ASSERT(m_display == nullptr);
		T_FATAL_ASSERT(m_grabbed == false);
	}

	virtual void destroy() T_OVERRIDE
	{
		releaseCapture();

		for (auto it : m_timers)
			Timers::getInstance().unbind(it.second);
		m_timers.clear();

		Timers::getInstance().dequeue();

		if (m_context != nullptr)
		{
			cairo_destroy(m_context);
			m_context = nullptr;
		}

		if (m_surface != nullptr)
		{
			cairo_surface_destroy(m_surface);
			m_surface = nullptr;
		}

		if (m_display != nullptr)
		{
			Assoc::getInstance().unbind(m_window);
			XDestroyWindow(m_display, m_window);
			m_display = nullptr;
			m_window = 0;
		}

		delete this;
	}

	virtual void setParent(IWidget* parent) T_OVERRIDE
	{
		Drawable parentWindow = (Drawable)parent->getInternalHandle();
		XReparentWindow(m_display, m_window, parentWindow, 0, 0);
	}

	virtual void setText(const std::wstring& text) T_OVERRIDE
	{
		m_text = text;
	}

	virtual std::wstring getText() const T_OVERRIDE
	{
		return m_text;
	}

	virtual void setForeground() T_OVERRIDE
	{
	}

	virtual bool isForeground() const T_OVERRIDE
	{
		return false;
	}

	virtual void setVisible(bool visible) T_OVERRIDE
	{
		if (visible != m_visible)
		{
			m_visible = visible;
			if (visible)
			{
				int32_t width = std::max< int32_t >(m_rect.getWidth(), 1);
				int32_t height = std::max< int32_t >(m_rect.getHeight(), 1);

				// Resize window.
				XMapWindow(m_display, m_window);
				XMoveResizeWindow(m_display, m_window, m_rect.left, m_rect.top, width, height);

				// Resize surface.
				cairo_xlib_surface_set_size(m_surface, width, height);
			}
			else
			{
				XUnmapWindow(m_display, m_window);
			}
		}
	}

	virtual bool isVisible() const T_OVERRIDE
	{
		return m_visible;
	}

	virtual void setActive() T_OVERRIDE
	{
	}

	virtual void setEnable(bool enable) T_OVERRIDE
	{
		m_enable = enable;
	}

	virtual bool isEnable() const T_OVERRIDE
	{
		return m_enable;
	}

	virtual bool hasFocus() const T_OVERRIDE
	{
		Window focusWindow; int revertTo;
		XGetInputFocus(m_display, &focusWindow, &revertTo);
		return bool(focusWindow == m_window);
	}

	virtual void setFocus() T_OVERRIDE
	{
		XSetInputFocus(m_display, m_window, RevertToNone, CurrentTime);
	}

	virtual bool hasCapture() const T_OVERRIDE
	{
		return m_grabbed;
	}

	virtual void setCapture() T_OVERRIDE
	{
		if (m_grabbed)
			return;
		
		m_grabbed = bool(XGrabPointer(
			m_display,
			m_window,
			False,
			ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
			GrabModeAsync,
			GrabModeAsync,
			None,
			None,
			CurrentTime
		) == GrabSuccess);

		m_grabbed = true;
	}

	virtual void releaseCapture() T_OVERRIDE
	{
		if (!m_grabbed)
			return;

		XUngrabPointer(m_display, CurrentTime);
		m_grabbed = false;
	}

	virtual void startTimer(int interval, int id) T_OVERRIDE
	{
		m_timers[id] = Timers::getInstance().bind(interval, [&](int32_t){
			TimerEvent timerEvent(m_owner, id);
			m_owner->raiseEvent(&timerEvent);			
		});
	}

	virtual void stopTimer(int id) T_OVERRIDE
	{
		auto it = m_timers.find(id);
		if (it != m_timers.end())
		{
			Timers::getInstance().unbind(it->second);
			m_timers.erase(it);
		}
	}

	virtual void setRect(const Rect& rect) T_OVERRIDE
	{
		int32_t oldWidth = std::max< int32_t >(m_rect.getWidth(), 1);
		int32_t oldHeight = std::max< int32_t >(m_rect.getHeight(), 1);

		int32_t newWidth = std::max< int32_t >(rect.getWidth(), 1);
		int32_t newHeight = std::max< int32_t >(rect.getHeight(), 1);

		m_rect = rect;

		if (m_visible)
		{
			if (newWidth != oldWidth || newHeight != oldHeight)
			{
				XMoveResizeWindow(m_display, m_window, rect.left, rect.top, newWidth, newHeight);
			 	cairo_xlib_surface_set_size(m_surface, newWidth, newHeight);
			}
			else
				XMoveWindow(m_display, m_window, rect.left, rect.top);
		}

		if (newWidth != oldWidth || newHeight != oldHeight)
		{
			SizeEvent sizeEvent(m_owner, m_rect.getSize());
			m_owner->raiseEvent(&sizeEvent);
		}
	}

	virtual Rect getRect() const T_OVERRIDE
	{
		return m_rect;
	}

	virtual Rect getInnerRect() const T_OVERRIDE
	{
		return Rect(0, 0, m_rect.getWidth(), m_rect.getHeight());
	}

	virtual Rect getNormalRect() const T_OVERRIDE
	{
		return Rect(0, 0, m_rect.getWidth(), m_rect.getHeight());
	}

	virtual void setFont(const Font& font) T_OVERRIDE
	{
		m_font = font;

		cairo_select_font_face(
			m_context,
			wstombs(m_font.getFace()).c_str(),
			CAIRO_FONT_SLANT_NORMAL,
			m_font.isBold() ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL
		);

		cairo_set_font_size(
			m_context,
			dpi96(m_font.getSize())
		);		
	}

	virtual Font getFont() const T_OVERRIDE
	{
		return m_font;
	}

	virtual const IFontMetric* getFontMetric() const T_OVERRIDE
	{
		return this;
	}

	virtual void setCursor(Cursor cursor) T_OVERRIDE
	{
	}

	virtual Point getMousePosition(bool relative) const T_OVERRIDE
	{
		Window root, child;
		int rootX, rootY;
		int winX, winY;
		unsigned int mask;

		XQueryPointer(
			m_display,
			relative ? m_window : DefaultRootWindow(m_display),
			&root,
			&child,
			&rootX, &rootY,
			&winX, &winY,
			&mask
		);

		return Point(winX, winY);
	}

	virtual Point screenToClient(const Point& pt) const T_OVERRIDE
	{
		Window dw; int x, y;
		XTranslateCoordinates(m_display, DefaultRootWindow(m_display), m_window, pt.x, pt.y, &x, &y, &dw);
		return Point(x, y);
	}

	virtual Point clientToScreen(const Point& pt) const T_OVERRIDE
	{
		Window dw; int x, y;
		XTranslateCoordinates(m_display, m_window, DefaultRootWindow(m_display), pt.x, pt.y, &x, &y, &dw);
		return Point(x, y);
	}

	virtual bool hitTest(const Point& pt) const T_OVERRIDE
	{
		return false;
	}

	virtual void setChildRects(const std::vector< IWidgetRect >& childRects) T_OVERRIDE
	{
		for (std::vector< IWidgetRect >::const_iterator i = childRects.begin(); i != childRects.end(); ++i)
		{
			if (i->widget)
				i->widget->setRect(i->rect);
		}
	}

	virtual Size getMinimumSize() const T_OVERRIDE
	{
		return Size(0, 0);
	}

	virtual Size getPreferedSize() const T_OVERRIDE
	{
		return Size(128, 64);
	}

	virtual Size getMaximumSize() const T_OVERRIDE
	{
		return Size(65535, 65535);
	}

	virtual void update(const Rect* rc, bool immediate) T_OVERRIDE
	{
		if (!m_pendingDraw)
		{
			m_pendingDraw = true;
			Timers::getInstance().queue([&]() {
				if (m_pendingDraw)
					draw();
				m_pendingDraw = false;
			});
		}
		else
		{
			draw();
			m_pendingDraw = false;
		}
	}

	virtual void* getInternalHandle() T_OVERRIDE
	{
		T_FATAL_ASSERT(m_window != 0);
		return (void*)m_window;
	}

	virtual SystemWindow getSystemWindow() T_OVERRIDE
	{
		return SystemWindow(m_display, m_window);
	}

	// IFontMetric

	virtual void getAscentAndDescent(int32_t& outAscent, int32_t& outDescent) const T_OVERRIDE
	{
		T_FATAL_ASSERT(m_surface != nullptr);
		
		cairo_font_extents_t x;
		cairo_font_extents(m_context, &x);

		outAscent = (int32_t)x.ascent;
		outDescent = (int32_t)x.descent;
	}

	virtual int32_t getAdvance(wchar_t ch, wchar_t next) const T_OVERRIDE
	{
		T_FATAL_ASSERT(m_surface != nullptr);
		
		cairo_text_extents_t tx;
		const char cs[] = { (char)ch, 0 };
		cairo_text_extents(m_context, cs, &tx);

		return (int32_t)tx.x_advance;
	}

	virtual int32_t getLineSpacing() const T_OVERRIDE
	{
		T_FATAL_ASSERT(m_surface != nullptr);
		
		cairo_font_extents_t x;
		cairo_font_extents(m_context, &x);

		return (int32_t)x.height;
	}

	virtual Size getExtent(const std::wstring& text) const T_OVERRIDE
	{
		T_FATAL_ASSERT(m_surface != nullptr);

		cairo_font_extents_t fx;
		cairo_text_extents_t tx;
		cairo_font_extents(m_context, &fx);
		cairo_text_extents(m_context, wstombs(text).c_str(), &tx);

		return Size(tx.width, fx.height);
	}

protected:
	enum
	{
		_NET_WM_STATE_REMOVE = 0,
		_NET_WM_STATE_ADD = 1,
		_NET_WM_STATE_TOGGLE = 2
	};

	EventSubject* m_owner;
	Display* m_display;
	int32_t m_screen;
	Drawable m_window;
	Rect m_rect;
	Font m_font;

	cairo_surface_t* m_surface;
	cairo_t* m_context;

	std::wstring m_text;
	bool m_visible;
	bool m_enable;
	bool m_grabbed;

	std::map< int32_t, int32_t > m_timers;

	int32_t m_lastMousePress;
	int32_t m_lastMouseButton;
	bool m_pendingDraw;

	bool create(IWidget* parent, Drawable window, const Rect& rect, bool visible)
	{
		if (window == 0)
			return false;

		m_window = window;
		m_rect = rect;
		m_visible = visible;

		XSelectInput(
			m_display,
			window, 
			ButtonPressMask |
			ButtonReleaseMask |
			StructureNotifyMask |
			KeyPressMask |
			ExposureMask |
			FocusChangeMask |
			PointerMotionMask
		);			

		if (visible)
	    	XMapWindow(m_display, window);

		XFlush(m_display);

		m_surface = cairo_xlib_surface_create(
			m_display,
			window,
			DefaultVisual(m_display, m_screen),
			m_rect.getWidth(),
			m_rect.getHeight()
		);

		m_context = cairo_create(m_surface);
		setFont(Font(L"Ubuntu Regular", 11));

		auto& a = Assoc::getInstance();

		a.bind(m_window, KeyPress, [&](XEvent& xe) {
			int nkeysyms;
			KeySym* ks = XGetKeyboardMapping(m_display, xe.xkey.keycode, 1, &nkeysyms);
			if (ks != nullptr)
			{
				VirtualKey vk = translateToVirtualKey(*ks);
				if (vk != VkNull)
				{
					KeyDownEvent keyDownEvent(m_owner, vk, xe.xkey.keycode, 0);
					m_owner->raiseEvent(&keyDownEvent);
				}

				char keybuf[8];
				int nk = XLookupString(&xe.xkey, keybuf, sizeof(keybuf), nullptr, nullptr);
				if (nk > 0)
				{
					KeyEvent keyEvent(m_owner, vk, xe.xkey.keycode, wchar_t(keybuf[0]));
					m_owner->raiseEvent(&keyEvent);
				}

				XFree(ks);
			}
		});

		a.bind(m_window, KeyRelease, [&](XEvent& xe) {
			int nkeysyms;
			KeySym* ks = XGetKeyboardMapping(m_display, xe.xkey.keycode, 1, &nkeysyms);
			if (ks != nullptr)
			{
				VirtualKey vk = translateToVirtualKey(*ks);
				if (vk != VkNull)
				{
					KeyUpEvent keyUpEvent(m_owner, vk, xe.xkey.keycode, 0);
					m_owner->raiseEvent(&keyUpEvent);
				}

				XFree(ks);
			}
		});

		a.bind(m_window, MotionNotify, [&](XEvent& xe){
			int32_t button = 0;
			if ((xe.xmotion.state & Button1Mask) != 0)
				button = MbtLeft;
			if ((xe.xmotion.state & Button2Mask) != 0)
				button = MbtRight;
			if ((xe.xmotion.state & Button3Mask) != 0)
				button = MbtMiddle;

			MouseMoveEvent mouseMoveEvent(
				m_owner,
				button,
				Point(xe.xmotion.x, xe.xmotion.y)
			);
			m_owner->raiseEvent(&mouseMoveEvent);
		});

		a.bind(m_window, ButtonPress, [&](XEvent& xe){
			int32_t button = 0;
			switch (xe.xbutton.button)
			{
			case Button1:
				button = MbtLeft;
				break;

			case Button2:
				button = MbtRight;
				break;

			case Button3:
				button = MbtMiddle;
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
			if (dbt <= 150 && m_lastMouseButton == button)
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
		});

		a.bind(m_window, ButtonRelease, [&](XEvent& xe){
			int32_t button = 0;
			switch (xe.xbutton.button)
			{
			case Button1:
				button = MbtLeft;
				break;

			case Button2:
				button = MbtRight;
				break;

			case Button3:
				button = MbtMiddle;
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

		if (parent == nullptr)
		{
			a.bind(m_window, ConfigureNotify, [&](XEvent& xe){
				m_rect = Rect(
					Point(xe.xconfigure.x, xe.xconfigure.y),
					Size(xe.xconfigure.width, xe.xconfigure.height)
				);

				int32_t newWidth = std::max< int32_t >(m_rect.getWidth(), 1);
				int32_t newHeight = std::max< int32_t >(m_rect.getHeight(), 1);

				if (m_visible)
					cairo_xlib_surface_set_size(m_surface, newWidth, newHeight);

				SizeEvent sizeEvent(m_owner, m_rect.getSize());
				m_owner->raiseEvent(&sizeEvent);
			});
		}

		a.bind(m_window, Expose, [&](XEvent& xe){
			update(nullptr, false);
		});

		return true;
	}

	void draw()
	{
		T_FATAL_ASSERT(m_surface != nullptr);

		cairo_push_group(m_context);

		CanvasX11 canvasImpl(m_context);
		Canvas canvas(&canvasImpl);
		PaintEvent paintEvent(
			m_owner,
			canvas,
			Rect(Point(0, 0), m_rect.getSize())
		);
		m_owner->raiseEvent(&paintEvent);

		cairo_pop_group_to_source(m_context);
		cairo_paint(m_context);

		cairo_surface_flush(m_surface);
	}

	void setWmProperty(const char* const property, int32_t value)
	{
		XEvent evt = { 0 };

		Atom atomWmState = XInternAtom(m_display, "_NET_WM_STATE", False);
		Atom atomWmProperty = XInternAtom(m_display, property, False);

		evt.type = ClientMessage;
		evt.xclient.window = m_window;
		evt.xclient.message_type = atomWmState;
		evt.xclient.format = 32;
		evt.xclient.data.l[0] = value;
		evt.xclient.data.l[1] = atomWmProperty;
		evt.xclient.data.l[2] = 0;
		evt.xclient.data.l[3] = 0;
		evt.xclient.data.l[4] = 0;

		XSendEvent(
			m_display,
			RootWindow(m_display, m_screen),
			False,
			SubstructureRedirectMask | SubstructureNotifyMask,
			&evt
		);
	}

	void setWmProperty(const char* const property1, const char* const property2, int32_t value)
	{
		XEvent evt = { 0 };

		Atom atomWmState = XInternAtom(m_display, "_NET_WM_STATE", False);
		Atom atomWmProperty1 = XInternAtom(m_display, property1, False);
		Atom atomWmProperty2 = XInternAtom(m_display, property2, False);

		evt.type = ClientMessage;
		evt.xclient.window = m_window;
		evt.xclient.message_type = atomWmState;
		evt.xclient.format = 32;
		evt.xclient.data.l[0] = value;
		evt.xclient.data.l[1] = atomWmProperty1;
		evt.xclient.data.l[2] = atomWmProperty2;
		evt.xclient.data.l[3] = 0;
		evt.xclient.data.l[4] = 0;

		XSendEvent(
			m_display,
			RootWindow(m_display, m_screen),
			False,
			SubstructureRedirectMask | SubstructureNotifyMask,
			&evt
		);
	}
};

	}
}

#endif	// traktor_ui_WidgetX11Impl_H
