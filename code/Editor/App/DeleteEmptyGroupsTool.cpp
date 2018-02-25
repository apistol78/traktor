/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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
			log::error << L"Unable to delete group" << Endl;
		group = 0;
	}
	else
	{
		for (RefArray< db::Group >::iterator i = childGroups.begin(); i != childGroups.end(); ++i)
			deleteEmptyGroups(*i);
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
	return 0;
}

bool DeleteEmptyGroupsTool::needOutputResources(std::set< Guid >& outDependencies) const
{
	return false;
}

bool DeleteEmptyGroupsTool::launch(ui::Widget* parent, IEditor* editor, const std::wstring& param)
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
