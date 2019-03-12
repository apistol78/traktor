#include "Render/Editor/SH/SHEditorPage.h"
#include "Render/Editor/SH/SHEditorPageFactory.h"
#include "Render/Editor/SH/SHStage.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.SHEditorPageFactory", 0, SHEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet SHEditorPageFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert< SHStage >();
	return typeSet;
}

bool SHEditorPageFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	outDependencies.insert(Guid(L"{5B786C6B-8818-A24A-BD1C-EE113B79BCE2}"));	// System/Primitive/Shaders/Primitive
	outDependencies.insert(Guid(L"{123602E4-BC6F-874D-92E8-A20852D140A3}"));	// System/Primitive/Textures/SmallFont
	return false;
}

Ref< editor::IEditorPage > SHEditorPageFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new  SHEditorPage(editor);
}

void SHEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

	}
}
