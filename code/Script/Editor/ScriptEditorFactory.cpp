#include "Script/Editor/ScriptEditorFactory.h"
#include "Script/Editor/ScriptEditor.h"
#include "Script/Script.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.script.ScriptEditorFactory", ScriptEditorFactory, editor::IObjectEditorFactory)

const TypeSet ScriptEditorFactory::getEditableTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< Script >());
	return typeSet;
}

editor::IObjectEditor* ScriptEditorFactory::createObjectEditor(editor::IEditor* editor) const
{
	return gc_new< ScriptEditor >(editor);
}

	}
}
