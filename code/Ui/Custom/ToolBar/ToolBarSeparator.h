#ifndef traktor_ui_custom_ToolBarSeparator_H
#define traktor_ui_custom_ToolBarSeparator_H

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

/*! \brief Tool bar separator.
 * \ingroup UIC
 */
class T_DLLCLASS ToolBarSeparator : public ToolBarItem
{
	T_RTTI_CLASS;

protected:
	virtual bool getToolTip(std::wstring& outToolTip) const T_OVERRIDE T_FINAL;

	virtual Size getSize(const ToolBar* toolBar, int imageWidth, int imageHeight) const T_OVERRIDE T_FINAL;

	virtual void paint(ToolBar* toolBar, Canvas& canvas, const Point& at, IBitmap* images, int imageWidth, int imageHeight) T_OVERRIDE T_FINAL;

	virtual bool mouseEnter(ToolBar* toolBar, MouseMoveEvent* mouseEvent) T_OVERRIDE T_FINAL;

	virtual void mouseLeave(ToolBar* toolBar, MouseMoveEvent* mouseEvent) T_OVERRIDE T_FINAL;

	virtual void buttonDown(ToolBar* toolBar, MouseButtonDownEvent* mouseEvent) T_OVERRIDE T_FINAL;

	virtual void buttonUp(ToolBar* toolBar, MouseButtonUpEvent* mouseEvent) T_OVERRIDE T_FINAL;
};

		}
	}
}

#endif	// traktor_ui_custom_ToolBarSeparator_H
