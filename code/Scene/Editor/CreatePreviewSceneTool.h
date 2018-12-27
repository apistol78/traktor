/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_scene_CreatePreviewSceneTool_H
#define traktor_scene_CreatePreviewSceneTool_H

#include "Editor/IWizardTool.h"

namespace traktor
{
	namespace scene
	{

class CreatePreviewSceneTool : public editor::IWizardTool
{
	T_RTTI_CLASS;

public:
	virtual std::wstring getDescription() const override final;

	virtual uint32_t getFlags() const override final;

	virtual bool launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance) override final;
};

	}
}

#endif	// traktor_scene_CreatePreviewSceneTool_H
