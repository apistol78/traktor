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

class Edit;
class ListView;
class PreviewList;
class TreeView;

	}

	namespace editor
	{

class NewInstanceDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	explicit NewInstanceDialog(PropertyGroup* settings);

	bool create(ui::Widget* parent, const std::wstring& initialGroup);

	virtual void destroy() override final;

	const TypeInfo* getType() const;

	const std::wstring& getInstanceName() const;

private:
	Ref< PropertyGroup > m_settings;
	Ref< ui::TreeView > m_categoryTree;
	Ref< ui::PreviewList > m_typeList;
	Ref< ui::Edit > m_editInstanceName;
	const TypeInfo* m_type;
	std::wstring m_instanceName;

	void updatePreviewList();

	void eventDialogClick(ui::ButtonClickEvent* event);

	void eventTreeItemSelected(ui::SelectionChangeEvent* event);
};

	}
}

