#include "Editor/App/DefaultObjectEditor.h"
#include "Editor/App/DefaultObjectEditorFactory.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.DefaultObjectEditorFactory", 0, DefaultObjectEditorFactory, IObjectEditorFactory)

const TypeInfoSet DefaultObjectEditorFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< ISerializable >());
	return typeSet;
}

bool DefaultObjectEditorFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return false;
}

Ref< IObjectEditor > DefaultObjectEditorFactory::createObjectEditor(IEditor* editor) const
{
	return new DefaultObjectEditor(editor);
}

void DefaultObjectEditorFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

	}
}
