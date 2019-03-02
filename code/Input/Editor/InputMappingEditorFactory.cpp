#include "Input/Editor/InputMappingAsset.h"
#include "Input/Editor/InputMappingEditor.h"
#include "Input/Editor/InputMappingEditorFactory.h"

namespace traktor
{
	namespace input
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.input.InputMappingEditorFactory", 0, InputMappingEditorFactory, editor::IEditorPageFactory)

const TypeInfoSet InputMappingEditorFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< InputMappingAsset >());
	return typeSet;
}

bool InputMappingEditorFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return false;
}

Ref< editor::IEditorPage > InputMappingEditorFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new InputMappingEditor(editor, site, document);
}

void InputMappingEditorFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

	}
}
