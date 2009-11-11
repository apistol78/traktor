#ifndef traktor_script_ScriptEditorFactory_H
#define traktor_script_ScriptEditorFactory_H

#include "Editor/IObjectEditorFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace script
	{

class T_DLLCLASS ScriptEditorFactory : public editor::IObjectEditorFactory
{
	T_RTTI_CLASS(ScriptEditorFactory)

public:
	virtual const TypeSet getEditableTypes() const;

	virtual Ref< editor::IObjectEditor > createObjectEditor(editor::IEditor* editor) const;
};

	}
}

#endif	// traktor_script_ScriptEditorFactory_H
