/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/SafeDestroy.h"
#include "Editor/App/SearchTool.h"
#include "Editor/App/SearchToolDialog.h"
#include "I18N/Text.h"
#include "Ui/StyleBitmap.h"

namespace traktor
{
	namespace editor
	{


T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.SearchTool", 0, SearchTool, IEditorTool)

std::wstring SearchTool::getDescription() const
{
	return i18n::Text(L"EDITOR_SEARCH_TOOL");
}

Ref< ui::IBitmap > SearchTool::getIcon() const
{
	return new ui::StyleBitmap(L"Editor.Tool.Search");
}

bool SearchTool::needOutputResources(std::set< Guid >& outDependencies) const
{
	return false;
}

bool SearchTool::launch(ui::Widget* parent, IEditor* editor, const std::wstring& param)
{
	safeDestroy(m_searchDialog);

	m_searchDialog = new SearchToolDialog(editor);
	if (m_searchDialog->create(parent))
		m_searchDialog->show();
	else
		safeDestroy(m_searchDialog);

	return true;
}

	}
}
