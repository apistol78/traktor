/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_MessageBoxWin32_H
#define traktor_ui_MessageBoxWin32_H

#include "Core/Misc/TString.h"
#include "Ui/Itf/IMessageBox.h"
#include "Ui/Win32/Window.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

/*! \brief
 * \ingroup UIW32
 */
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
