#include <X11/Xutil.h>
#include "Ui/Events/CloseEvent.h"
#include "Ui/X11/FormX11.h"

namespace traktor
{
	namespace ui
	{

FormX11::FormX11(EventSubject* owner, Display* display, int32_t screen)
:	WidgetX11Impl< IForm >(owner, display, screen)
{
}

bool FormX11::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	T_FATAL_ASSERT(parent == nullptr);

	const int32_t c_minWidth = 16;
	const int32_t c_minHeight = 16;

	width = std::max< int32_t >(width, c_minWidth);
	height = std::max< int32_t >(height, c_minHeight);

	Window window = XCreateWindow(
		m_display,
		DefaultRootWindow(m_display),
		0,
		0,
		width,
		height,
		0,
		0,
		InputOutput,
		CopyFromParent,
		0,
		nullptr
	);

	// Notify WM about form title.
	std::string cs = wstombs(text);
	const char* csp = cs.c_str();

	XTextProperty tp;
	XStringListToTextProperty((char**)&csp, 1, &tp);

	XSetWMName(m_display, window, &tp);

	// Register "delete window" window manager message.
	m_atomWmDeleteWindow = XInternAtom(m_display, "WM_DELETE_WINDOW", False);

	Assoc::getInstance().bind(window, ClientMessage, [&](XEvent& xe){
		if ((Atom)xe.xclient.data.l[0] == m_atomWmDeleteWindow)
		{
			CloseEvent closeEvent(m_owner);
			m_owner->raiseEvent(&closeEvent);
			if (!(closeEvent.consumed() && closeEvent.cancelled()))
				destroy();
		}		
	});

	XSetWMProtocols(m_display, window, &m_atomWmDeleteWindow, 1);

	return WidgetX11Impl< IForm >::create(nullptr, window, Rect(0, 0, width, height), false);
}

void FormX11::setIcon(ISystemBitmap* icon)
{
}

void FormX11::maximize()
{
}

void FormX11::minimize()
{
}

void FormX11::restore()
{
}

bool FormX11::isMaximized() const
{
	return false;
}

bool FormX11::isMinimized() const
{
	return false;
}

void FormX11::hideProgress()
{
}

void FormX11::showProgress(int32_t current, int32_t total)
{
}

	}
}

