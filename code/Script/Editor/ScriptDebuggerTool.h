#ifndef traktor_script_ScriptDebuggerTool_H
#define traktor_script_ScriptDebuggerTool_H

#include "Editor/IEditorTool.h"

namespace traktor
{
	namespace ui
	{
	
class CloseEvent;

	}

	namespace script
	{

class ScriptDebuggerDialog;

class ScriptDebuggerTool : public editor::IEditorTool
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const T_OVERRIDE T_FINAL;

	virtual Ref< ui::IBitmap > getIcon() const T_OVERRIDE T_FINAL;

	virtual bool launch(ui::Widget* parent, editor::IEditor* editor) T_OVERRIDE T_FINAL;

private:
	Ref< ScriptDebuggerDialog > m_debuggerDialog;

	void eventCloseDialog(ui::CloseEvent* event);
};

	}
}

#endif	// traktor_script_ScriptDebuggerTool_H
