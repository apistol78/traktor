#ifndef traktor_ui_custom_ToolBar_H
#define traktor_ui_custom_ToolBar_H

#include "Core/RefArray.h"
#include "Ui/Command.h"
#include "Ui/Widget.h"

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

class Bitmap;

		namespace custom
		{

class ToolTip;
class ToolBarItem;

/*! \brief Tool bar control.
 * \ingroup UIC
 */
class T_DLLCLASS ToolBar : public Widget
{
	T_RTTI_CLASS;

public:
	enum WidgetStyles
	{
		WsUnderline = (WsUser)
	};

	ToolBar();

	bool create(Widget* parent, int style = WsUnderline);

	void destroy();

	uint32_t addImage(Bitmap* image, uint32_t imageCount);

	uint32_t addItem(ToolBarItem* item);

	Ref< ToolBarItem > getItem(uint32_t id);

	Ref< ToolBarItem > getItem(const Point& at);

	void addClickEventHandler(EventHandler* eventHandler);

	virtual Size getPreferedSize() const;

private:
	Ref< ToolTip > m_toolTip;
	int m_style;
	Ref< Bitmap > m_imageEnabled;
	Ref< Bitmap > m_imageDisabled;
	uint32_t m_imageWidth;
	uint32_t m_imageHeight;
	uint32_t m_imageCount;
	RefArray< ToolBarItem > m_items;
	Ref< ToolBarItem > m_trackItem;

	void eventMouseMove(Event* event);

	void eventButtonDown(Event* event);

	void eventButtonUp(Event* event);

	void eventPaint(Event* event);

	void eventShowTip(Event* event);
};

		}
	}
}

#endif	// traktor_ui_custom_ToolBar_H
