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
