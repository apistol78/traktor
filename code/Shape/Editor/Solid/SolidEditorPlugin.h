#pragma once

#include "Scene/Editor/ISceneEditorPlugin.h"

namespace traktor
{
	namespace shape
	{

class SolidEditorPlugin : public scene::ISceneEditorPlugin
{
	T_RTTI_CLASS;

public:
	SolidEditorPlugin(scene::SceneEditorContext* context);

	virtual bool create(ui::Widget* parent, ui::ToolBar* toolBar) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

private:
	scene::SceneEditorContext* m_context;
};

	}
}
