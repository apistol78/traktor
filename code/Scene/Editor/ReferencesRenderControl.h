#ifndef traktor_scene_ReferencesRenderControl_H
#define traktor_scene_ReferencesRenderControl_H

#include "Scene/Editor/ISceneRenderControl.h"

namespace traktor
{
	namespace ui
	{

class Widget;

	}

	namespace scene
	{

class SceneEditorContext;

class ReferencesRenderControl : public ISceneRenderControl
{
	T_RTTI_CLASS(ReferencesRenderControl)

public:
	ReferencesRenderControl();

	bool create(ui::Widget* parent, SceneEditorContext* context);

	void destroy();

	virtual void setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings);

	virtual bool handleCommand(const ui::Command& command);

	virtual void update();
};

	}
}

#endif	// traktor_scene_ReferencesRenderControl_H
