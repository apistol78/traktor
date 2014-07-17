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
	virtual bool getToolTip(std::wstring& outToolTip) const;

	virtual Size getSize(const ToolBar* toolBar, int imageWidth, int imageHeight) const;

	virtual void paint(ToolBar* toolBar, Canvas& canvas, const Point& at, Bitmap* images, int imageWidth, int imageHeight);

	virtual bool mouseEnter(ToolBar* toolBar, MouseMoveEvent* mouseEvent);

	virtual void mouseLeave(ToolBar* toolBar, MouseMoveEvent* mouseEvent);

	virtual void buttonDown(ToolBar* toolBar, MouseButtonDownEvent* mouseEvent);

	virtual void buttonUp(ToolBar* toolBar, MouseButtonUpEvent* mouseEvent);
};

		}
	}
}

#endif	// traktor_ui_custom_ToolBarSeparator_H
