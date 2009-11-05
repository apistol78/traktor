#ifndef traktor_scene_ISceneRenderControl_H
#define traktor_scene_ISceneRenderControl_H

#include "Core/Object.h"

namespace traktor
{
	namespace ui
	{

class Command;

	}

	namespace scene
	{

class ISceneRenderControl : public Object
{
	T_RTTI_CLASS(ISceneRenderControl)

public:
	virtual void destroy() = 0;

	virtual void updateWorldRenderer() = 0;

	virtual bool handleCommand(const ui::Command& command) = 0;

	virtual void update() = 0;
};

	}
}

#endif	// traktor_scene_ISceneRenderControl_H
