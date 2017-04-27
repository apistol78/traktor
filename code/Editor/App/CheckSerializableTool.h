/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_CheckSerializableTool_H
#define traktor_editor_CheckSerializableTool_H

#include "Editor/IEditorTool.h"

namespace traktor
{
	namespace editor
	{

class CheckSerializableTool : public IEditorTool
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const T_OVERRIDE T_FINAL;

	virtual Ref< ui::IBitmap > getIcon() const T_OVERRIDE T_FINAL;

	virtual bool launch(ui::Widget* parent, IEditor* editor, const std::wstring& param) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_editor_CheckSerializableTool_H
