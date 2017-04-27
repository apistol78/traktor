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

class MenuBarWin32;

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

	/*! \brief Bar registration.
	 *
	 * As some widgets need to be notified about
	 * parent form's size we need to register those widgets
	 * explicitly.
	 * The form also needs to know about these widgets
	 * in order to calculate it's inner size.
	 */
	//@{

	void registerMenuBar(MenuBarWin32* menuBar);

	void unregisterMenuBar(MenuBarWin32* menuBar);

	//@}

private:
	MenuBarWin32* m_menuBar;
	ComRef< ITaskbarList3 > m_taskBarList;

	LRESULT eventInitMenuPopup(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);

	LRESULT eventMenuCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);

	LRESULT eventClose(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);

	LRESULT eventDestroy(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);

	LRESULT eventTaskBarButtonCreated(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, bool& pass);
};

	}
}

#endif	// traktor_ui_FormWin32_H
