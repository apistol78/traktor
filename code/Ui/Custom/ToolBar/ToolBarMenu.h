/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_ToolBarMenu_H
#define traktor_ui_custom_ToolBarMenu_H

#include "Core/RefArray.h"
#include "Ui/Command.h"
#include "Ui/Point.h"
#include "Ui/Custom/ToolBar/ToolBarItem.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class MenuItem;
class PopupMenu;

		namespace custom
		{

/*! \brief Tool bar menu item.
 * \ingroup UIC
 */
class T_DLLCLASS ToolBarMenu : public ToolBarItem
{
	T_RTTI_CLASS;

public:
	ToolBarMenu(const std::wstring& text, const std::wstring& toolTip);

	int32_t add(MenuItem* item);

	bool remove(int32_t index);

	void removeAll();

	int32_t count() const;

	MenuItem* get(int32_t index) const;

protected:
	virtual bool getToolTip(std::wstring& outToolTip) const T_OVERRIDE T_FINAL;

	virtual Size getSize(const ToolBar* toolBar, int imageWidth, int imageHeight) const T_OVERRIDE T_FINAL;

	virtual void paint(ToolBar* toolBar, Canvas& canvas, const Point& at, IBitmap* images, int imageWidth, int imageHeight) T_OVERRIDE T_FINAL;

	virtual bool mouseEnter(ToolBar* toolBar, MouseMoveEvent* mouseEvent) T_OVERRIDE T_FINAL;

	virtual void mouseLeave(ToolBar* toolBar, MouseMoveEvent* mouseEvent) T_OVERRIDE T_FINAL;

	virtual void buttonDown(ToolBar* toolBar, MouseButtonDownEvent* mouseEvent) T_OVERRIDE T_FINAL;

	virtual void buttonUp(ToolBar* toolBar, MouseButtonUpEvent* mouseEvent) T_OVERRIDE T_FINAL;

private:
	Command m_command;
	std::wstring m_text;
	std::wstring m_toolTip;
	RefArray< MenuItem > m_items;
	bool m_hover;
	Point m_menuPosition;
	Ref< PopupMenu > m_menu;
};

		}
	}
}

#endif	// traktor_ui_custom_ToolBarMenu_H
