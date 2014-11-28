#ifndef traktor_script_ScriptProfilerTool_H
#define traktor_script_ScriptProfilerTool_H

#include "Editor/IEditorTool.h"

namespace traktor
{
	namespace script
	{

class ScriptProfilerDialog;

class ScriptProfilerTool : public editor::IEditorTool
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const;

	virtual bool launch(ui::Widget* parent, editor::IEditor* editor);

private:
	Ref< ScriptProfilerDialog > m_profilerDialog;
};

	}
}

#endif	// traktor_script_ScriptProfilerTool_H
