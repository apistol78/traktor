/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_NewWorkspaceDialog_H
#define traktor_editor_NewWorkspaceDialog_H

#include "Ui/ConfigDialog.h"

namespace traktor
{
	namespace ui
	{

class Edit;

		namespace custom
		{

class PreviewList;

		}
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
	Ref< ui::custom::PreviewList > m_templateList;
	Ref< ui::Edit > m_editName;
	Ref< ui::Edit > m_editPath;
	std::wstring m_workspacePath;

	void eventBrowsePathButtonClick(ui::ButtonClickEvent* event);

	void eventDialogClick(ui::ButtonClickEvent* event);
};

	}
}

#endif	// traktor_editor_SettingsDialog_H
