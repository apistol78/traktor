#ifndef traktor_ui_custom_ToolBarEmbed_H
#define traktor_ui_custom_ToolBarEmbed_H

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

class Widget;

		namespace custom
		{

/*! \brief Tool bar embedded widget.
 * \ingroup UIC
 */
class T_DLLCLASS ToolBarEmbed : public ToolBarItem
{
	T_RTTI_CLASS(ToolBarEmbed)

public:
	ToolBarEmbed(Widget* widget, int width);

protected:
	virtual bool getToolTip(std::wstring& outToolTip) const;

	virtual Size getSize(const ToolBar* toolBar, int imageWidth, int imageHeight) const;

	virtual void paint(ToolBar* toolBar, Canvas& canvas, const ui::Point& at, ui::Bitmap* images, int imageWidth, int imageHeight);

	virtual void mouseEnter(ToolBar* toolBar, MouseEvent* mouseEvent);

	virtual void mouseLeave(ToolBar* toolBar, MouseEvent* mouseEvent);

	virtual void buttonDown(ToolBar* toolBar, MouseEvent* mouseEvent);

	virtual void buttonUp(ToolBar* toolBar, MouseEvent* mouseEvent);

private:
	Ref< Widget > m_widget;
	int m_width;
};

		}
	}
}

#endif	// traktor_ui_custom_ToolBarEmbed_H
