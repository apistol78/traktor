#ifndef traktor_script_ScriptEditor_H
#define traktor_script_ScriptEditor_H

#include "Core/Heap/Ref.h"
#include "Editor/IObjectEditor.h"
#include "Script/ScriptContext.h"

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

class IEditor;

	}

	namespace ui
	{

class Event;
class ListBox;

		namespace custom
		{

class Splitter;
class SyntaxRichEdit;
class StatusBar;

		}
	}

	namespace script
	{

class Script;
class ScriptManager;
class ScriptContext;

class T_DLLCLASS ScriptEditor
:	public editor::IObjectEditor
,	public script::IErrorCallback
{
	T_RTTI_CLASS(ScriptEditor)

public:
	ScriptEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, Serializable* object);

	virtual void destroy();

	virtual void apply();

private:
	editor::IEditor* m_editor;
	Ref< Script > m_script;
	Ref< ScriptManager > m_scriptManager;
	Ref< ScriptContext > m_scriptContext;
	Ref< ui::custom::Splitter > m_splitter;
	Ref< ui::ListBox > m_dependencyList;
	Ref< ui::custom::SyntaxRichEdit > m_edit;
	Ref< ui::custom::StatusBar > m_compileStatus;
	int32_t m_compileCountDown;

	virtual void syntaxError(uint32_t line, const std::wstring& message);

	virtual void otherError(const std::wstring& message);

	void updateDependencyList();

	void eventDependencyToolClick(ui::Event* event);

	void eventDependencyListDoubleClick(ui::Event* event);

	void eventScriptChange(ui::Event* event);

	void eventTimer(ui::Event* event);
};

	}
}

#endif	// traktor_script_ScriptEditor_H
