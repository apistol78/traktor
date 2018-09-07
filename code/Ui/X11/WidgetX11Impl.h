#ifndef traktor_ui_WidgetX11Impl_H
#define traktor_ui_WidgetX11Impl_H

#include <map>
#include <X11/Xlib.h>
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
	,	m_visible(false)
	,	m_enable(true)
	,	m_lastMousePress(0)
	,	m_lastMouseButton(0)
	,	m_grabbed(false)
	,	m_drawPending(false)
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
	}

	virtual void setVisible(bool visible) T_OVERRIDE
	{
		if (visible != m_visible)
		{
			m_visible = visible;
			if (visible)
			{
				XMapWindow(m_display, m_window);
				XFlush(m_display);

				int32_t width = std::max< int32_t >(m_rect.getWidth(), 1);
				int32_t height = std::max< int32_t >(m_rect.getHeight(), 1);

				// Resize window.
				XMoveWindow(m_display, m_window, m_rect.left, m_rect.top);
				XResizeWindow(m_display, m_window, width, height);
				XFlush(m_display);

				// Resize surface.
				cairo_xlib_surface_set_size(m_surface, width, height);
			}
			else
			{
				XUnmapWindow(m_display, m_window);
				XFlush(m_display);
			}
		}
	}

	virtual bool isVisible(bool includingParents) const T_OVERRIDE
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
		
		// m_grabbed = bool(XGrabPointer(
		// 	m_display,
		// 	m_window,
		// 	True,
		// 	0,
		// 	GrabModeAsync,
		// 	GrabModeAsync,
		// 	m_window,
		// 	None,
		// 	CurrentTime
		// ) == GrabSuccess);

		m_grabbed = true;
	}

	virtual void releaseCapture() T_OVERRIDE
	{
		if (!m_grabbed)
			return;

		//XUngrabPointer(m_display, CurrentTime);
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
		m_rect = rect;

		int32_t width = std::max< int32_t >(rect.getWidth(), 1);
		int32_t height = std::max< int32_t >(rect.getHeight(), 1);

		// Resize window.
		XMoveWindow(m_display, m_window, rect.left, rect.top);
		XResizeWindow(m_display, m_window, width, height);

		// Resize surface.
		cairo_xlib_surface_set_size(m_surface, width, height);

		XFlush(m_display);
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
		return Point(0, 0);
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
		if (!immediate)
		{
			if (!m_drawPending)
			{
				Timers::getInstance().queue([&](){
					m_drawPending = false;
					draw();
				});
				m_drawPending = true;
			}
		}
		else
			draw();
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

		cairo_t* ctx = cairo_create(m_surface);

		cairo_select_font_face(
			ctx,
			wstombs(m_font.getFace()).c_str(),
			CAIRO_FONT_SLANT_NORMAL,
			m_font.isBold() ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL
		);
		cairo_set_font_size(
			ctx,
			dpi96(m_font.getSize())
		);

		cairo_font_extents(ctx, &x);
		cairo_destroy(ctx);

		outAscent = x.ascent;
		outDescent = x.descent;
	}

	virtual int32_t getAdvance(wchar_t ch, wchar_t next) const T_OVERRIDE
	{
		return 0;
	}

	virtual int32_t getLineSpacing() const T_OVERRIDE
	{
		return 0;
	}

	virtual Size getExtent(const std::wstring& text) const T_OVERRIDE
	{
		T_FATAL_ASSERT(m_surface != nullptr);

		cairo_font_extents_t fx;
		cairo_text_extents_t tx;

		cairo_t* ctx = cairo_create(m_surface);

		cairo_select_font_face(
			ctx,
			wstombs(m_font.getFace()).c_str(),
			CAIRO_FONT_SLANT_NORMAL,
			m_font.isBold() ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL
		);
		cairo_set_font_size(
			ctx,
			dpi96(m_font.getSize())
		);

		cairo_font_extents(ctx, &fx);
		cairo_text_extents(ctx, wstombs(text).c_str(), &tx);
		cairo_destroy(ctx);

		return Size(tx.width, fx.height);
	}

protected:
	EventSubject* m_owner;
	Display* m_display;
	int32_t m_screen;
	Drawable m_window;
	Rect m_rect;
	Font m_font;
	cairo_surface_t* m_surface;
	std::wstring m_text;
	bool m_visible;
	bool m_enable;
	std::map< int32_t, int32_t > m_timers;
	int32_t m_lastMousePress;
	int32_t m_lastMouseButton;
	bool m_grabbed;
	bool m_drawPending;

	bool create(IWidget* parent, Drawable window, int32_t width, int32_t height, bool visible)
	{
		if (window == 0)
			return false;

		m_window = window;

    	XSelectInput(
			m_display,
			window, 
			ButtonPressMask |
			ButtonReleaseMask |
			SubstructureNotifyMask |
			StructureNotifyMask |
			SubstructureRedirectMask |
			KeyPressMask |
			ExposureMask |
			FocusChangeMask |
			PointerMotionMask
		);

		if (visible)
	    	XMapWindow(m_display, window);

		m_visible = visible;

		XFlush(m_display);

		m_surface = cairo_xlib_surface_create(
			m_display,
			window,
			DefaultVisual(m_display, m_screen),
			width,
			height
		);
		cairo_xlib_surface_set_size(m_surface, width, height);

		m_rect = Rect(0, 0, width, height);
		m_font = Font(L"Ubuntu Regular", 11);

		auto& a = Assoc::getInstance();

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

		a.bind(m_window, ConfigureNotify, [&](XEvent& xe){
			m_rect.setWidth(xe.xconfigure.width);
			m_rect.setHeight(xe.xconfigure.height);

			cairo_xlib_surface_set_size(
				m_surface,
				m_rect.getWidth(),
				m_rect.getHeight()
			);

			SizeEvent sizeEvent(m_owner, m_rect.getSize());
			m_owner->raiseEvent(&sizeEvent);
		});

		a.bind(m_window, Expose, [&](XEvent& xe){
			draw();
		});

		return true;
	}

	void draw()
	{
		T_FATAL_ASSERT(m_surface != nullptr);

		if (!m_visible)
			return;

		cairo_t* ctx = cairo_create(m_surface);
		cairo_push_group(ctx);

		CanvasX11 canvasImpl(ctx);
		canvasImpl.setFont(m_font);

		Canvas canvas(&canvasImpl);
		PaintEvent paintEvent(
			m_owner,
			canvas,
			m_rect
		);
		m_owner->raiseEvent(&paintEvent);

		cairo_pop_group_to_source(ctx);
		cairo_paint(ctx);
		cairo_destroy(ctx);
	}	
};

	}
}

#endif	// traktor_ui_WidgetX11Impl_H

