#ifndef traktor_ui_custom_EditList_H
#define traktor_ui_custom_EditList_H

#include "Ui/Edit.h"
#include "Ui/ListBox.h"

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

class T_DLLCLASS EditList : public ListBox
{
	T_RTTI_CLASS;

public:
	enum WidgetStyles
	{
		WsAutoAdd = (WsUser << 3),
		WsAutoRemove = (WsUser << 4)
	};

	EditList();

	bool create(Widget* parent, int style = ListBox::WsDefault);

private:
	Ref< Edit > m_editItem;
	int32_t m_editId;
	bool m_autoAdd;
	bool m_autoRemove;

	void eventDoubleClick(MouseDoubleClickEvent* event);

	void eventEditFocus(FocusEvent* event);
};

		}
	}
}

#endif	// EditList_H
