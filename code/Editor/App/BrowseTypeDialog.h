/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Ui/ConfigDialog.h"

namespace traktor
{

class PropertyGroup;

	namespace ui
	{

class PreviewList;
class TreeView;
class TreeViewItem;

	}

	namespace editor
	{

class BrowseTypeDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	BrowseTypeDialog(PropertyGroup* settings);

	bool create(ui::Widget* parent, const TypeInfoSet* base, bool onlyEditable, bool onlyInstantiable);

	virtual void destroy() override final;

	const TypeInfo* getSelectedType() const;

private:
	Ref< PropertyGroup > m_settings;
	Ref< ui::TreeView > m_categoryTree;
	Ref< ui::PreviewList > m_typeList;
	const TypeInfo* m_type;

	void updatePreviewList();

	void eventDialogClick(ui::ButtonClickEvent* event);

	void eventTreeItemSelected(ui::SelectionChangeEvent* event);

	void eventListDoubleClick(ui::MouseDoubleClickEvent* event);
};

	}
}

