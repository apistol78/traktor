#ifndef traktor_ui_custom_ComboBox_H
#define traktor_ui_custom_ComboBox_H

#include <string>
#include "Ui/Widget.h"

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

class Button;
class Edit;
class ListBox;

		namespace custom
		{

/*! \brief Combo box control.
 * \ingroup UIC
 */
class T_DLLCLASS ComboBox : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, const std::wstring& text, int style = WsNone);

	int add(const std::wstring& item);

	bool remove(int index);

	void removeAll();

	int count() const;

	std::wstring get(int index) const;
	
	void select(int index);

	int getSelected() const;

	std::wstring getSelectedItem() const;

private:
	Ref< Button > m_buttonArrow;
	Ref< Edit > m_edit;
	Ref< ListBox > m_listBox;

	void eventSize(SizeEvent* event);
};

		}
	}
}

#endif	// traktor_ui_custom_ComboBox_H
