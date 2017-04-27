/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_DialogWin32_H
#define traktor_ui_DialogWin32_H

#include "Ui/Itf/IDialog.h"
#include "Ui/Win32/WidgetWin32Impl.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class DialogWin32 : public WidgetWin32Impl< IDialog >
{
public:
	DialogWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style);

	virtual void setIcon(drawing::Image* icon);
	
	virtual int showModal();

	virtual void endModal(int result);

	virtual void setMinSize(const Size& minSize);

	virtual void setVisible(bool visible);

private:
	bool m_modal;
	Size m_minSize;
	bool m_centerDesktop;
	int32_t m_result;

	LRESULT eventInitDialog(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);

	LRESULT eventSizing(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);

	LRESULT eventClose(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);

	LRESULT eventEndModal(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& skip);
};

	}
}

#endif	// traktor_ui_DialogWin32_H
