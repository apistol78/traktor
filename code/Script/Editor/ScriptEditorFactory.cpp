#include "Script/Editor/Script.h"
#include "Script/Editor/ScriptEditor.h"
#include "Script/Editor/ScriptEditorFactory.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptEditorFactory", 0, ScriptEditorFactory, editor::IObjectEditorFactory)

const TypeInfoSet ScriptEditorFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Script >());
	return typeSet;
}

Ref< editor::IObjectEditor > ScriptEditorFactory::createObjectEditor(editor::IEditor* editor) const
{
	return new ScriptEditor(editor);
}

	}
}
