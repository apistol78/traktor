#ifndef traktor_ui_DropDown_H
#define traktor_ui_DropDown_H

#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! \brief DropDown
 * \ingroup UI
 */
class T_DLLCLASS DropDown : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, const std::wstring& text = L"", int style = WsBorder);
	
	int add(const std::wstring& item);

	bool remove(int index);

	void removeAll();

	int count() const;

	std::wstring get(int index) const;

	void select(int index);

	int getSelected() const;

	std::wstring getSelectedItem() const;

	void addSelectEventHandler(EventHandler* eventHandler);
};

	}
}

#endif	// traktor_ui_DropDown_H
