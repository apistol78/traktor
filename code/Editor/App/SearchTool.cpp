/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
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

bool SearchTool::launch(ui::Widget* parent, IEditor* editor, const PropertyGroup* param)
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
