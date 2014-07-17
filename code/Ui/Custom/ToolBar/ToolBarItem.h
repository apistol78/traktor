#ifndef traktor_ui_custom_ToolBarItem_H
#define traktor_ui_custom_ToolBarItem_H

#include "Core/Object.h"
#include "Ui/Associative.h"
#include "Ui/Size.h"

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

class Canvas;
class Point;
class Bitmap;
class MouseButtonDownEvent;
class MouseButtonUpEvent;
class MouseMoveEvent;

		namespace custom
		{

class ToolBar;

/*! \brief Tool bar item.
 * \ingroup UIC
 */
class T_DLLCLASS ToolBarItem
:	public Object
,	public Associative
{
	T_RTTI_CLASS;

public:
	ToolBarItem();

	void setEnable(bool enable);

	bool isEnable() const;

protected:
	friend class ToolBar;

	virtual bool getToolTip(std::wstring& outToolTip) const = 0;

	virtual Size getSize(const ToolBar* toolBar, int imageWidth, int imageHeight) const = 0;

	virtual void paint(ToolBar* toolBar, Canvas& canvas, const Point& at, Bitmap* images, int imageWidth, int imageHeight) = 0;

	/*! \brief Mouse enter item.
	 *
	 * \return True if tracking of item desired; false will not cause mouse to be captured.
	 */
	virtual bool mouseEnter(ToolBar* toolBar, MouseMoveEvent* mouseEvent) = 0;

	virtual void mouseLeave(ToolBar* toolBar, MouseMoveEvent* mouseEvent) = 0;

	virtual void buttonDown(ToolBar* toolBar, MouseButtonDownEvent* mouseEvent) = 0;

	virtual void buttonUp(ToolBar* toolBar, MouseButtonUpEvent* mouseEvent) = 0;

private:
	bool m_enable;
};

		}
	}
}

#endif	// traktor_ui_custom_ToolBarItem_H
