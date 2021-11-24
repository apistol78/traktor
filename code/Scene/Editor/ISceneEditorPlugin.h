#pragma once

#include "Core/Object.h"
#include "Ui/Command.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class ToolBar;
class Widget;

	}

	namespace scene
	{

class SceneEditorContext;

/*! Scene editor plugin interface.
 * \ingroup Scene
 */
class T_DLLCLASS ISceneEditorPlugin : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool create(ui::Widget* parent, ui::ToolBar* toolBar) = 0;

	virtual bool handleCommand(const ui::Command& command) = 0;
};

	}
}

