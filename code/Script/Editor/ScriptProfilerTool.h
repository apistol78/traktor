/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_script_ScriptProfilerTool_H
#define traktor_script_ScriptProfilerTool_H

#include "Editor/IEditorTool.h"

namespace traktor
{
	namespace ui
	{

class CloseEvent;

	}

	namespace script
	{

class ScriptProfilerDialog;

class ScriptProfilerTool : public editor::IEditorTool
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const override final;

	virtual Ref< ui::IBitmap > getIcon() const override final;

	virtual bool needOutputResources(std::set< Guid >& outDependencies) const override final;

	virtual bool launch(ui::Widget* parent, editor::IEditor* editor, const std::wstring& param) override final;

private:
	Ref< ScriptProfilerDialog > m_profilerDialog;

	void eventCloseDialog(ui::CloseEvent* event);
};

	}
}

#endif	// traktor_script_ScriptProfilerTool_H
