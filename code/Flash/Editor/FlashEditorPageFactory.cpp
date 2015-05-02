#include "Flash/Editor/FlashEditorPage.h"
#include "Flash/Editor/FlashEditorPageFactory.h"
#include "Flash/Editor/FlashMovieAsset.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashEditorPageFactory", 0, FlashEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet FlashEditorPageFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< FlashMovieAsset >());
	return typeSet;
}

bool FlashEditorPageFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	outDependencies.insert(Guid(L"{666412C3-5A4E-1B47-87D3-BC0165D2DC26}"));	// System/SystemAssets
	return true;
}

Ref< editor::IEditorPage > FlashEditorPageFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new FlashEditorPage(editor, site, document);
}

void FlashEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"Flash.Editor.Rewind"));
	outCommands.push_back(ui::Command(L"Flash.Editor.Play"));
	outCommands.push_back(ui::Command(L"Flash.Editor.Stop"));
	outCommands.push_back(ui::Command(L"Flash.Editor.Forward"));
}

	}
}
