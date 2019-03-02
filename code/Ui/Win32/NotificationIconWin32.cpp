#include "Core/Misc/TString.h"
#include "Ui/EventSubject.h"
#include "Ui/Events/MouseButtonDownEvent.h"
#include "Ui/Events/MouseButtonUpEvent.h"
#include "Ui/Events/MouseDoubleClickEvent.h"
#include "Ui/Win32/BitmapWin32.h"
#include "Ui/Win32/NotificationIconWin32.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

uint32_t s_taskbarCreated = 0;

		}

NotificationIconWin32::NotificationIconWin32(EventSubject* owner)
:	m_owner(owner)
{
}

bool NotificationIconWin32::create(const std::wstring& text, ISystemBitmap* image)
{
	if (!m_hWnd.create(
		NULL,
		_T("TraktorWin32Class"),
		_T(""),
		0,
		0,
		0,
		0,
		0,
		0
	))
		return false;

	m_hWnd.registerMessageHandler(WM_USER + 1, new MethodMessageHandler< NotificationIconWin32 >(this, &NotificationIconWin32::eventNotification));

	std::memset(&m_nid, 0, sizeof(m_nid));
	m_nid.cbSize = sizeof(NOTIFYICONDATA);
	m_nid.hWnd = m_hWnd;
	m_nid.uID = 1;
	m_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	m_nid.uCallbackMessage = WM_USER + 1;
	m_nid.hIcon = reinterpret_cast< BitmapWin32* >(image)->createIcon();
	_tcscpy_s(m_nid.szTip, sizeof_array(m_nid.szTip), wstots(text).c_str());
	if (!Shell_NotifyIcon(NIM_ADD, &m_nid))
		return false;

	s_taskbarCreated = RegisterWindowMessage(_T("TaskbarCreated"));
	m_hWnd.registerMessageHandler(s_taskbarCreated, new MethodMessageHandler< NotificationIconWin32 >(this, &NotificationIconWin32::eventTaskbarCreated));

	return true;
}

void NotificationIconWin32::destroy()
{
	Shell_NotifyIcon(NIM_DELETE, &m_nid);
	delete this;
}

void NotificationIconWin32::setImage(ISystemBitmap* image)
{
	T_ASSERT (image);
	m_nid.hIcon = reinterpret_cast< BitmapWin32* >(image)->createIcon();
	Shell_NotifyIcon(NIM_MODIFY, &m_nid);
}

LRESULT NotificationIconWin32::eventNotification(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
	POINT pnt;
	GetCursorPos(&pnt);

	switch (lParam)
	{
	case WM_LBUTTONDBLCLK:
		{
			MouseDoubleClickEvent m(
				m_owner,
				MbtLeft,
				Point(pnt.x, pnt.y)
			);
			m_owner->raiseEvent(&m);
			if (!m.consumed())
				pass = true;
		}
		break;

	case WM_RBUTTONDBLCLK:
		{
			MouseDoubleClickEvent m(
				m_owner,
				MbtRight,
				Point(pnt.x, pnt.y)
			);
			m_owner->raiseEvent(&m);
			if (!m.consumed())
				pass = true;
		}
		break;

	case WM_LBUTTONDOWN:
		{
			MouseButtonDownEvent m(
				m_owner,
				MbtLeft,
				Point(pnt.x, pnt.y)
			);
			m_owner->raiseEvent(&m);
			if (!m.consumed())
				pass = true;
		}
		break;

	case WM_RBUTTONDOWN:
		{
			MouseButtonDownEvent m(
				m_owner,
				MbtRight,
				Point(pnt.x, pnt.y)
			);
			m_owner->raiseEvent(&m);
			if (!m.consumed())
				pass = true;
		}
		break;

	case WM_LBUTTONUP:
		{
			MouseButtonUpEvent m(
				m_owner,
				MbtLeft,
				Point(pnt.x, pnt.y)
			);
			m_owner->raiseEvent(&m);
			if (!m.consumed())
				pass = true;
		}
		break;

	case WM_RBUTTONUP:
		{
			MouseButtonUpEvent m(
				m_owner,
				MbtRight,
				Point(pnt.x, pnt.y)
			);
			m_owner->raiseEvent(&m);
			if (!m.consumed())
				pass = true;
		}
		break;

	default:
		pass = true;
	}
	return 0;
}

LRESULT NotificationIconWin32::eventTaskbarCreated(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass)
{
	// Task bar created; most possibly after a explorer crash.
	Shell_NotifyIcon(NIM_ADD, &m_nid);
	return 0;
}

	}
}
