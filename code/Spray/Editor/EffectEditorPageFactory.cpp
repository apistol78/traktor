#include "Spray/EffectData.h"
#include "Spray/Editor/EffectEditorPage.h"
#include "Spray/Editor/EffectEditorPageFactory.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.EffectEditorPageFactory", 0, EffectEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet EffectEditorPageFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< EffectData >());
	return typeSet;
}

bool EffectEditorPageFactory::needOutputResources(const TypeInfo& typeInfo) const
{
	return true;
}

Ref< editor::IEditorPage > EffectEditorPageFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new EffectEditorPage(editor, site, document);
}

void EffectEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"Effect.Editor.Rewind"));
	outCommands.push_back(ui::Command(L"Effect.Editor.Play"));
	outCommands.push_back(ui::Command(L"Effect.Editor.Stop"));
	outCommands.push_back(ui::Command(L"Effect.Editor.Reset"));
	outCommands.push_back(ui::Command(L"Effect.Editor.ToggleGuide"));
	outCommands.push_back(ui::Command(L"Effect.Editor.ToggleVelocity"));
	outCommands.push_back(ui::Command(L"Effect.Editor.RandomizeSeed"));
}

	}
}
