#include "Amalgam/Editor/Target.h"
#include "Amalgam/Editor/TargetEditor.h"
#include "Amalgam/Editor/TargetEditorFactory.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.TargetEditorFactory", 0, TargetEditorFactory, editor::IObjectEditorFactory)

const TypeInfoSet TargetEditorFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Target >());
	return typeSet;
}

bool TargetEditorFactory::needOutputResources(const TypeInfo& typeInfo) const
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
