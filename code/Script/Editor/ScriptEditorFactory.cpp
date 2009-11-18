#include "Script/Editor/ScriptEditorFactory.h"
#include "Script/Editor/ScriptEditor.h"
#include "Script/Script.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.script.ScriptEditorFactory", ScriptEditorFactory, editor::IObjectEditorFactory)

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
