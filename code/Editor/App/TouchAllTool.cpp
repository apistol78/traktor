#include "Core/Log/Log.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Editor/IEditor.h"
#include "Editor/App/TouchAllTool.h"
#include "I18N/Text.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.TouchAllTool", 0, TouchAllTool, IEditorTool)

std::wstring TouchAllTool::getDescription() const
{
	return i18n::Text(L"EDITOR_TOUCH_ALL");
}

Ref< ui::IBitmap > TouchAllTool::getIcon() const
{
	return 0;
}

bool TouchAllTool::launch(ui::Widget* parent, IEditor* editor, const std::wstring& param)
{
	Ref< db::Database > database = editor->getSourceDatabase();
	if (!database)
		return true;

	RefArray< db::Instance > instances;
	db::recursiveFindChildInstances(
		database->getRootGroup(),
		db::FindInstanceAll(),
		instances
	);

	int32_t errorCount = 0;
	for (RefArray< db::Instance >::const_iterator i = instances.begin(); i != instances.end(); ++i)
	{
		if (!(*i)->checkout())
		{
			log::error << L"Unable to checkout " << (*i)->getPath() << L"." << Endl;
			errorCount++;
			continue;
		}

		Ref< ISerializable > object = (*i)->getObject();
		if (!object)
		{
			log::error << L"Unable to get object from " << (*i)->getPath() << L"." << Endl;
			(*i)->revert();
			errorCount++;
			continue;
		}

		(*i)->setObject(object);

		if (!(*i)->commit())
		{
			(*i)->revert();
			log::error << L"Unable to commit " << (*i)->getPath() << L"." << Endl;
			errorCount++;
		}
	}

	log::info << L"All instances touched, " << errorCount << L" error(s)." << Endl;
	return true;
}

	}
}
