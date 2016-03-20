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
	virtual std::wstring getDescription() const T_OVERRIDE T_FINAL;

	virtual Ref< ui::IBitmap > getIcon() const T_OVERRIDE T_FINAL;

	virtual bool launch(ui::Widget* parent, editor::IEditor* editor) T_OVERRIDE T_FINAL;

private:
	Ref< ScriptProfilerDialog > m_profilerDialog;
};

	}
}

#endif	// traktor_script_ScriptProfilerTool_H
