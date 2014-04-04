#ifndef traktor_ui_ToolForm_H
#define traktor_ui_ToolForm_H

#include "Ui/Container.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! \brief Tool form.
 * \ingroup UI
 */
class T_DLLCLASS ToolForm : public Container
{
	T_RTTI_CLASS;

public:
	enum StyleFlags
	{
		WsDefault = WsResizable | WsSystemBox | WsCloseBox | WsCaption
	};

	bool create(Widget* parent, const std::wstring& text, int width, int height, int style = WsDefault, Layout* layout = 0);

	void center();

	void addCloseEventHandler(EventHandler* eventHandler);

	void addNcButtonDownEventHandler(EventHandler* eventHandler);

	void addNcButtonUpEventHandler(EventHandler* eventHandler);

	void addNcDoubleClickEventHandler(EventHandler* eventHandler);

	void addNcMouseMoveEventHandler(EventHandler* eventHandler);

	void addNcMouseWheelEventHandler(EventHandler* eventHandler);

	virtual bool acceptLayout() const;
};

	}
}

#endif	// traktor_ui_ToolForm_H
