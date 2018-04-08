/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_model_ModelTool_H
#define traktor_model_ModelTool_H

#include <Editor/IEditorTool.h>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MODEL_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace model
	{

class ModelToolDialog;

class T_DLLCLASS ModelTool : public editor::IEditorTool
{
	T_RTTI_CLASS;

public:
	virtual ~ModelTool();

	virtual std::wstring getDescription() const T_OVERRIDE T_FINAL;

	virtual Ref< ui::IBitmap > getIcon() const T_OVERRIDE T_FINAL;

	virtual bool needOutputResources(std::set< Guid >& outDependencies) const T_OVERRIDE T_FINAL;

	virtual bool launch(ui::Widget* parent, editor::IEditor* editor, const std::wstring& param) T_OVERRIDE T_FINAL;

private:
	Ref< ModelToolDialog > m_dialog;
};

	}
}

#endif	// traktor_model_ModelTool_H
