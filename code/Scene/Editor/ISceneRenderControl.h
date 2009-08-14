#ifndef traktor_scene_ISceneRenderControl_H
#define traktor_scene_ISceneRenderControl_H

#include "Ui/Widget.h"

namespace traktor
{
	namespace world
	{

class WorldRenderSettings;

	}

	namespace ui
	{

class Command;

	}

	namespace scene
	{

class ISceneRenderControl : public ui::Widget
{
	T_RTTI_CLASS(ISceneRenderControl)

public:
	virtual void setWorldRenderSettings(world::WorldRenderSettings* worldRenderSettings) = 0;

	virtual bool handleCommand(const ui::Command& command) = 0;
};

	}
}

#endif	// traktor_scene_ISceneRenderControl_H
