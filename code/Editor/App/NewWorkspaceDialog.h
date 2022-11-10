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
	namespace ui
	{

class Edit;
class PreviewList;

	}

	namespace editor
	{

class NewWorkspaceDialog : public ui::ConfigDialog
{
	T_RTTI_CLASS;

public:
	bool create(ui::Widget* parent);

	const std::wstring& getWorkspacePath() const;

private:
	Ref< ui::PreviewList > m_templateList;
	Ref< ui::Edit > m_editName;
	Ref< ui::Edit > m_editPath;
	Ref< ui::Edit > m_editWorkspaceFile;
	std::wstring m_workspacePath;

	void updateWorkspaceFile();

	void eventBrowsePathButtonClick(ui::ButtonClickEvent* event);

	void eventDialogClick(ui::ButtonClickEvent* event);
};

	}
}
