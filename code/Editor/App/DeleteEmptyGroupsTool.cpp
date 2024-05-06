/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Editor/IEditor.h"
#include "Editor/App/DeleteEmptyGroupsTool.h"
#include "I18N/Text.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

void deleteEmptyGroups(db::Group* group)
{
	RefArray< db::Group > childGroups;
	RefArray< db::Instance > childInstances;

	group->getChildGroups(childGroups);
	group->getChildInstances(childInstances);

	if (childGroups.empty() && childInstances.empty())
	{
		log::info << L"Deleting empty group \"" << group->getPath() << L"\"..." << Endl;
		if (!group->remove())
			log::error << L"Unable to delete group." << Endl;
	}
	else
	{
		for (auto childGroup : childGroups)
			deleteEmptyGroups(childGroup);
	}
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.DeleteEmptyGroupsTool", 0, DeleteEmptyGroupsTool, IEditorTool)

std::wstring DeleteEmptyGroupsTool::getDescription() const
{
	return i18n::Text(L"EDITOR_DELETE_EMPTY_GROUPS");
}

Ref< ui::IBitmap > DeleteEmptyGroupsTool::getIcon() const
{
	return nullptr;
}

bool DeleteEmptyGroupsTool::needOutputResources(std::set< Guid >& outDependencies) const
{
	return false;
}

bool DeleteEmptyGroupsTool::launch(ui::Widget* parent, IEditor* editor, const PropertyGroup* param)
{
	db::Database* db = editor->getSourceDatabase();
	if (!db)
		return false;

	deleteEmptyGroups(db->getRootGroup());

	log::info << L"Done" << Endl;
	return true;
}

	}
}
