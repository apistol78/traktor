#include "Ui/Win32/MessageBoxWin32.h"
#include "Ui/Itf/IWidget.h"

namespace traktor
{
	namespace ui
	{

MessageBoxWin32::MessageBoxWin32(EventSubject* owner)
:	m_hWndParent(NULL)
,	m_type(0)
{
}

bool MessageBoxWin32::create(IWidget* parent, const std::wstring& message, const std::wstring& caption, int style)
{
	m_hWndParent = parent ? (HWND)parent->getInternalHandle() : NULL;

	m_message = wstots(message);
	m_caption = wstots(caption);

	if (style & MbOk)
		m_type |= MB_OK;
	if (style & MbCancel)
		m_type |= MB_OKCANCEL;
	if (style & MbYesNo)
		m_type |= MB_YESNO;
	if (style & MbIconExclamation)
		m_type |= MB_ICONEXCLAMATION;
	if (style & MbIconHand)
		m_type |= MB_ICONHAND;
	if (style & MbIconError)
		m_type |= MB_ICONERROR;
	if (style & MbIconQuestion)
		m_type |= MB_ICONQUESTION;
	if (style & MbIconInformation)
		m_type |= MB_ICONINFORMATION;
	if (style & MbStayOnTop)
		m_type |= MB_TOPMOST;

	if (!parent)
		m_type |= MB_TOPMOST | MB_SETFOREGROUND | MB_SYSTEMMODAL;

	return true;
}

void MessageBoxWin32::destroy()
{
}

int MessageBoxWin32::showModal()
{
	int result = MessageBox(
		m_hWndParent,
		m_message.c_str(),
		m_caption.c_str(),
		m_type
	);

	if (result == IDOK)
		return DrOk;
	if (result == IDYES)
		return DrYes;

	return DrCancel;
}

	}
}
