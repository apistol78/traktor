#ifndef traktor_script_ScriptDebuggerTool_H
#define traktor_script_ScriptDebuggerTool_H

#include "Editor/IEditorTool.h"

namespace traktor
{
	namespace script
	{

class ScriptDebuggerDialog;

class ScriptDebuggerTool : public editor::IEditorTool
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const;

	virtual bool launch(ui::Widget* parent, editor::IEditor* editor);

private:
	Ref< ScriptDebuggerDialog > m_debuggerDialog;
};

	}
}

#endif	// traktor_script_ScriptDebuggerTool_H
