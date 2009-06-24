#include "Spray/Editor/EffectEditorPageFactory.h"
#include "Spray/Editor/EffectEditorPage.h"
#include "Spray/Effect.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.spray.EffectEditorPageFactory", EffectEditorPageFactory, editor::IEditorPageFactory)

const TypeSet EffectEditorPageFactory::getEditableTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< Effect >());
	return typeSet;
}

editor::IEditorPage* EffectEditorPageFactory::createEditorPage(editor::IEditor* editor) const
{
	return gc_new< EffectEditorPage >(editor);
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
