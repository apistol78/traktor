/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_QuickOpenDialog_H
#define traktor_editor_QuickOpenDialog_H

#include "Ui/Dialog.h"

namespace traktor
{
	namespace db
	{

class Instance;

	}

	namespace ui
	{

class Edit;

		namespace custom
		{

class GridView;

		}
	}

	namespace editor
	{

class IEditor;

class QuickOpenDialog : public ui::Dialog
{
	T_RTTI_CLASS;

public:
	QuickOpenDialog(IEditor* editor);

	bool create(ui::Widget* parent);

	db::Instance* showDialog();

private:
	IEditor* m_editor;
	Ref< ui::Edit > m_editFilter;
	Ref< ui::custom::GridView > m_gridSuggestions;
	RefArray< db::Instance > m_instances;

	void updateSuggestions(const std::wstring& filter);

	void eventFilterChange(ui::ContentChangeEvent* event);

	void eventFilterKey(ui::KeyDownEvent* event);

	void eventSuggestionSelect(ui::SelectionChangeEvent* event);
};

	}
}

#endif	// traktor_editor_QuickOpenDialog_H
