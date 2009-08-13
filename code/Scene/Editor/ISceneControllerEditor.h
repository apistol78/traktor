#ifndef traktor_scene_ISceneControllerEditor_H
#define traktor_scene_ISceneControllerEditor_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Container;
class Command;

	}

	namespace scene
	{

class SceneEditorContext;

/*! \brief
 */
class T_DLLCLASS ISceneControllerEditor : public Object
{
	T_RTTI_CLASS(ISceneControllerEditor)

public:
	virtual bool create(SceneEditorContext* context, ui::Container* parent) = 0;

	virtual void destroy() = 0;

	virtual void propertiesChanged() = 0;

	virtual bool handleCommand(const ui::Command& command) = 0;
};

	}
}

#endif	// traktor_scene_ISceneControllerEditor_H
