#ifndef traktor_ui_MessageBoxWin32_H
#define traktor_ui_MessageBoxWin32_H

#include "Ui/Win32/Window.h"
#include "Ui/Itf/IMessageBox.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

class MessageBoxWin32 : public IMessageBox
{
public:
	MessageBoxWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& message, const std::wstring& caption, int style);

	virtual void destroy();

	virtual int showModal();

private:
	HWND m_hWndParent;
	tstring m_message;
	tstring m_caption;
	UINT m_type;
};

	}
}

#endif	// traktor_ui_MessageBoxWin32_H
