/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_SearchTool_H
#define traktor_editor_SearchTool_H

#include "Editor/IEditorTool.h"

namespace traktor
{
	namespace editor
	{

class SearchToolDialog;

class SearchTool : public IEditorTool
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const T_OVERRIDE T_FINAL;

	virtual Ref< ui::IBitmap > getIcon() const T_OVERRIDE T_FINAL;

	virtual bool needOutputResources(std::set< Guid >& outDependencies) const T_OVERRIDE T_FINAL;

	virtual bool launch(ui::Widget* parent, IEditor* editor, const std::wstring& param) T_OVERRIDE T_FINAL;

private:
	Ref< SearchToolDialog > m_searchDialog;
};

	}
}

#endif	// traktor_editor_SearchTool_H
