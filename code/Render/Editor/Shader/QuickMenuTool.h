#ifndef traktor_render_QuickMenuTool_H
#define traktor_render_QuickMenuTool_H

#include "Ui/Dialog.h"

namespace traktor
{
	namespace ui
	{

class Edit;

		namespace custom
		{

class ListBox;

		}
	}

	namespace render
	{

class QuickMenuTool : public ui::Dialog
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	const TypeInfo* showMenu();

private:
	Ref< ui::Edit > m_editFilter;
	Ref< ui::custom::ListBox > m_listBoxSuggestions;

	void updateSuggestions(const std::wstring& filter);

	void eventFilterChange(ui::ContentChangeEvent* event);

	void eventFilterKey(ui::KeyDownEvent* event);

	void eventSuggestionSelect(ui::SelectionChangeEvent* event);
};

	}
}

#endif	// traktor_render_QuickMenuTool_H
