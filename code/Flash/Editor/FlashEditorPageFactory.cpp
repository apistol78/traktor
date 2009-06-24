#include "Flash/Editor/FlashEditorPageFactory.h"
#include "Flash/Editor/FlashEditorPage.h"
#include "Flash/Editor/FlashMovieAsset.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.flash.FlashEditorPageFactory", FlashEditorPageFactory, editor::IEditorPageFactory)

const TypeSet FlashEditorPageFactory::getEditableTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< FlashMovieAsset >());
	return typeSet;
}

editor::IEditorPage* FlashEditorPageFactory::createEditorPage(editor::IEditor* editor) const
{
	return gc_new< FlashEditorPage >(editor);
}

void FlashEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"Flash.Editor.Rewind"));
	outCommands.push_back(ui::Command(L"Flash.Editor.Play"));
	outCommands.push_back(ui::Command(L"Flash.Editor.Stop"));
	outCommands.push_back(ui::Command(L"Flash.Editor.Forward"));
	outCommands.push_back(ui::Command(L"Flash.Editor.LayoutCenter"));
	outCommands.push_back(ui::Command(L"Flash.Editor.LayoutAspect"));
}

	}
}
