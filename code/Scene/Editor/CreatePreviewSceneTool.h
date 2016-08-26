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
	virtual std::wstring getDescription() const T_OVERRIDE T_FINAL;

	virtual uint32_t getFlags() const T_OVERRIDE T_FINAL;

	virtual bool launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_scene_CreatePreviewSceneTool_H
