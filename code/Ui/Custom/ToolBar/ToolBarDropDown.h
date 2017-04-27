/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_custom_ToolBarDropDown_H
#define traktor_ui_custom_ToolBarDropDown_H

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
		namespace custom
		{

/*! \brief Tool bar dropdown.
 * \ingroup UIC
 */
class T_DLLCLASS ToolBarDropDown : public ToolBarItem
{
	T_RTTI_CLASS;

public:
	ToolBarDropDown(const Command& command, int32_t width, const std::wstring& toolTip);

	int32_t add(const std::wstring& item);

	bool remove(int32_t index);

	void removeAll();

	int32_t count() const;

	std::wstring get(int32_t index) const;

	void select(int32_t index);

	int32_t getSelected() const;

	std::wstring getSelectedItem() const;

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
	int32_t m_width;
	std::wstring m_toolTip;
	std::vector< std::wstring > m_items;
	int32_t m_selected;
	bool m_hover;
	int32_t m_dropPosition;
	Point m_menuPosition;
};

		}
	}
}

#endif	// traktor_ui_custom_ToolBarDropDown_H
