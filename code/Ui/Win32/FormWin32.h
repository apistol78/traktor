/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_FormWin32_H
#define traktor_ui_FormWin32_H

#include "Ui/Itf/IForm.h"
#include "Ui/Win32/WidgetWin32Impl.h"

namespace traktor
{
	namespace ui
	{

/*! \brief
 * \ingroup UIW32
 */
class FormWin32 : public WidgetWin32Impl< IForm >
{
public:
	FormWin32(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setIcon(ISystemBitmap* icon) T_OVERRIDE T_FINAL;

	virtual void maximize() T_OVERRIDE T_FINAL;

	virtual void minimize() T_OVERRIDE T_FINAL;

	virtual void restore() T_OVERRIDE T_FINAL;

	virtual bool isMaximized() const T_OVERRIDE T_FINAL;

	virtual bool isMinimized() const T_OVERRIDE T_FINAL;

	virtual void hideProgress() T_OVERRIDE T_FINAL;

	virtual void showProgress(int32_t current, int32_t total) T_OVERRIDE T_FINAL;

private:
	ComRef< ITaskbarList3 > m_taskBarList;
	HWND m_hWndLastFocus;

	LRESULT eventClose(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);

	LRESULT eventDestroy(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);

	LRESULT eventActivate(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);

	LRESULT eventTaskBarButtonCreated(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);
};

	}
}

#endif	// traktor_ui_FormWin32_H
