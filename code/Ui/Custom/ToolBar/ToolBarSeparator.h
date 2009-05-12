#ifndef traktor_ui_custom_ToolBarSeparator_H
#define traktor_ui_custom_ToolBarSeparator_H

#include "Core/Heap/Ref.h"
#include "Ui/Custom/ToolBar/ToolBarItem.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
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
	T_RTTI_CLASS(ToolBarSeparator)

protected:
	virtual bool getToolTip(std::wstring& outToolTip) const;

	virtual Size getSize(const ToolBar* toolBar, int imageWidth, int imageHeight) const;

	virtual void paint(ToolBar* toolBar, Canvas& canvas, const ui::Point& at, ui::Bitmap* images, int imageWidth, int imageHeight);

	virtual void mouseEnter(ToolBar* toolBar, MouseEvent* mouseEvent);

	virtual void mouseLeave(ToolBar* toolBar, MouseEvent* mouseEvent);

	virtual void buttonDown(ToolBar* toolBar, MouseEvent* mouseEvent);

	virtual void buttonUp(ToolBar* toolBar, MouseEvent* mouseEvent);
};

		}
	}
}

#endif	// traktor_ui_custom_ToolBarSeparator_H
