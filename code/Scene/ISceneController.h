#ifndef traktor_scene_ISceneController_H
#define traktor_scene_ISceneController_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace scene
	{

class Scene;

class T_DLLCLASS ISceneController : public Object
{
	T_RTTI_CLASS;

public:
	virtual void update(Scene* scene, float time, float deltaTime) = 0;
};

	}
}

#endif	// traktor_scene_ISceneController_H
