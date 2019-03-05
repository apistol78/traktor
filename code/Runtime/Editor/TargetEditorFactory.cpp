#include "Runtime/Editor/TargetEditor.h"
#include "Runtime/Editor/TargetEditorFactory.h"
#include "Runtime/Editor/Deploy/Target.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.TargetEditorFactory", 0, TargetEditorFactory, editor::IObjectEditorFactory)

const TypeInfoSet TargetEditorFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Target >());
	return typeSet;
}

bool TargetEditorFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return false;
}

Ref< editor::IObjectEditor > TargetEditorFactory::createObjectEditor(editor::IEditor* editor) const
{
	return new TargetEditor(editor);
}

void TargetEditorFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

	}
}
