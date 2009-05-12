#ifndef traktor_script_ScriptEditor_H
#define traktor_script_ScriptEditor_H

#include "Core/Heap/Ref.h"
#include "Editor/ObjectEditor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class SyntaxRichEdit;

		}
	}

	namespace script
	{

class Script;

class T_DLLCLASS ScriptEditor : public editor::ObjectEditor
{
	T_RTTI_CLASS(ScriptEditor)

public:
	virtual bool create(ui::Widget* parent, db::Instance* instance, Object* object);

	virtual void destroy();

	virtual void apply();

private:
	Ref< Script > m_script;
	Ref< ui::custom::SyntaxRichEdit > m_edit;
};

	}
}

#endif	// traktor_script_ScriptEditor_H
