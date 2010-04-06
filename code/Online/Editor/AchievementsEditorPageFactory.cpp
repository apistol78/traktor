#include "Online/Editor/AchievementsAsset.h"
#include "Online/Editor/AchievementsEditorPage.h"
#include "Online/Editor/AchievementsEditorPageFactory.h"

namespace traktor
{
	namespace online
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.online.AchievementsEditorPageFactory", 0, AchievementsEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet AchievementsEditorPageFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< AchievementsAsset >());
	return typeSet;
}

Ref< editor::IEditorPage > AchievementsEditorPageFactory::createEditorPage(editor::IEditor* editor) const
{
	return new AchievementsEditorPage(editor);
}

void AchievementsEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

	}
}
