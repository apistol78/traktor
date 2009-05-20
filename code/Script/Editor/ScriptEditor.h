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
	namespace editor
	{

class Editor;

	}

	namespace ui
	{

class Event;
class ListBox;

		namespace custom
		{

class Splitter;
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
	ScriptEditor(editor::Editor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, Object* object);

	virtual void destroy();

	virtual void apply();

private:
	editor::Editor* m_editor;
	Ref< Script > m_script;
	Ref< ui::custom::Splitter > m_splitter;
	Ref< ui::ListBox > m_dependencyList;
	Ref< ui::custom::SyntaxRichEdit > m_edit;

	void updateDependencyList();

	void eventDependencyToolClick(ui::Event* event);

	void eventDependencyListDoubleClick(ui::Event* event);
};

	}
}

#endif	// traktor_script_ScriptEditor_H
