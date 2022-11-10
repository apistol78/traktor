/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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
class GridView;

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
	Ref< ui::GridView > m_gridSuggestions;
	RefArray< db::Instance > m_instances;

	void updateSuggestions(const std::wstring& filter);

	void eventFilterChange(ui::ContentChangeEvent* event);

	void eventFilterKey(ui::KeyDownEvent* event);

	void eventSuggestionSelect(ui::SelectionChangeEvent* event);
};

	}
}

