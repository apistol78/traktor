#include "Ui/Events/ContentChangeEvent.h"
#include "Ui/Win32/RichEditWin32.h"

namespace traktor
{
	namespace ui
	{

RichEditWin32::RichEditWin32(EventSubject* owner)
:	WidgetWin32Impl< IRichEdit >(owner)
{
}

bool RichEditWin32::create(IWidget* parent, const std::wstring& text, int style)
{
	UINT nativeStyle, nativeStyleEx;
	getNativeStyles(style, nativeStyle, nativeStyleEx);

#if !defined(WINCE)
	LoadLibrary(L"riched20.dll");

	if (!m_hWnd.create(
		(HWND)parent->getInternalHandle(),
		RICHEDIT_CLASS,
		text.c_str(),
		WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL |
		ES_WANTRETURN | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_LEFT | ES_MULTILINE | nativeStyle,
		nativeStyleEx,
		0,
		0,
		0,
		0,
		0,
		true
	))
		return false;

	if (!WidgetWin32Impl::create(style))
		return false;

	m_hWnd.sendMessage(EM_SETEVENTMASK, 0, ENM_CHANGE);
#else
	if (!m_hWnd.create(
		(HWND)parent->getInternalHandle(),
		L"EDIT",
		text.c_str(),
		WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL |
		ES_WANTRETURN | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_LEFT | ES_MULTILINE | nativeStyle,
		nativeStyleEx,
		0,
		0,
		0,
		0,
		0,
		true
	))
		return false;

	if (!WidgetWin32Impl::create(style))
		return false;
#endif

	m_hWnd.registerMessageHandler(WM_GETDLGCODE, new MethodMessageHandler< RichEditWin32 >(this, &RichEditWin32::onGetDlgCode));
	m_hWnd.registerMessageHandler(WM_REFLECTED_COMMAND, new MethodMessageHandler< RichEditWin32 >(this, &RichEditWin32::onCommand));

	return true;
}

int RichEditWin32::addAttribute(const Color4ub& textColor, const Color4ub& backColor, bool bold, bool italic, bool underline)
{
#if !defined(WINCE)
	CHARFORMAT2 cf;

	std::memset(&cf, 0, sizeof(cf));
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR | CFM_BACKCOLOR | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE;
	cf.dwEffects = (bold ? CFE_BOLD : 0) | (italic ? CFE_ITALIC : 0) | (underline ? CFE_UNDERLINE : 0);
	cf.crTextColor = textColor.getABGR() & 0x00ffffff;
	cf.crBackColor = backColor.getABGR() & 0x00ffffff;
	m_attributes.push_back(cf);

	return int(m_attributes.size() - 1);
#else
	return 0;
#endif
}

void RichEditWin32::setAttribute(int start, int length, int attribute)
{
#if !defined(WINCE)
	CHARRANGE set = { start, start + length };
	CHARRANGE tmp;

	m_hWnd.sendMessage(EM_SETEVENTMASK, 0, 0);
	m_hWnd.sendMessage(EM_HIDESELECTION, TRUE, 0);
	m_hWnd.sendMessage(EM_EXGETSEL, 0, (LPARAM)&tmp);
	m_hWnd.sendMessage(EM_EXSETSEL, 0, (LPARAM)&set);
	m_hWnd.sendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&m_attributes[attribute]);
	m_hWnd.sendMessage(EM_EXSETSEL, 0, (LPARAM)&tmp);
	m_hWnd.sendMessage(EM_HIDESELECTION, FALSE, 0);
	m_hWnd.sendMessage(EM_SETEVENTMASK, 0, ENM_CHANGE);
#endif
}

void RichEditWin32::clear(bool attributes, bool content)
{
#if !defined(WINCE)
	if (attributes)
		m_attributes.resize(0);
#endif
	if (content)
		setText(L"");
}

void RichEditWin32::insert(const std::wstring& text)
{
#if !defined(WINCE)
	SETTEXTEX st;	
	st.flags = ST_SELECTION;
	st.codepage = CP_ACP;
	m_hWnd.sendMessage(EM_SETTEXTEX, (WPARAM)&st, (LPARAM)text.c_str());
#endif
}

int RichEditWin32::getCaretOffset() const
{
#if !defined(WINCE)
	CHARRANGE sel;
	m_hWnd.sendMessage(EM_EXGETSEL, 0, (LPARAM)&sel);
	return sel.cpMin;
#else
	return 0;
#endif
}

int RichEditWin32::getLineFromOffset(int offset) const
{
#if !defined(WINCE)
	return int(m_hWnd.sendMessage(EM_EXLINEFROMCHAR, 0, (LPARAM)offset));
#else
	return 0;
#endif
}

int RichEditWin32::getLineCount() const
{
	return int(m_hWnd.sendMessage(EM_GETLINECOUNT, 0, 0));
}

int RichEditWin32::getLineOffset(int line) const
{
	return int(m_hWnd.sendMessage(EM_LINEINDEX, (WPARAM)line, 0));
}

int RichEditWin32::getLineLength(int line) const
{
	int characterIndex = getLineOffset(line);
	return int(m_hWnd.sendMessage(EM_LINELENGTH, (WPARAM)characterIndex, 0));
}

std::wstring RichEditWin32::getLine(int line) const
{
	wchar_t buf[1024];
	*reinterpret_cast< WORD* >(buf) = sizeof(buf);
	
	LRESULT count = m_hWnd.sendMessage(EM_GETLINE, (WPARAM)line, (LPARAM)buf);
	buf[count] = L'\0';

	return buf;
}

bool RichEditWin32::redo()
{
#if !defined(WINCE)
	return bool(m_hWnd.sendMessage(EM_REDO, 0, 0) == TRUE);
#else
	return false;
#endif
}

bool RichEditWin32::undo()
{
	return bool(m_hWnd.sendMessage(EM_UNDO, 0, 0) == TRUE);
}

LRESULT RichEditWin32::onGetDlgCode(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip)
{
	return DLGC_WANTALLKEYS;
}

LRESULT RichEditWin32::onCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip)
{
	if ((HWND)lParam == m_hWnd && HIWORD(wParam) == EN_CHANGE)
	{
		ContentChangeEvent contentChangeEvent(m_owner);
		m_owner->raiseEvent(&contentChangeEvent);
	}
	return TRUE;
}

	}
}
