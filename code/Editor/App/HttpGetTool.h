/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_HttpGetTool_H
#define traktor_editor_HttpGetTool_H

#include "Editor/IEditorTool.h"

namespace traktor
{
	namespace editor
	{

class HttpGetTool : public IEditorTool
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const override final;

	virtual Ref< ui::IBitmap > getIcon() const override final;

	virtual bool needOutputResources(std::set< Guid >& outDependencies) const override final;

	virtual bool launch(ui::Widget* parent, IEditor* editor, const std::wstring& param) override final;
};

	}
}

#endif	// traktor_editor_HttpGetTool_H
