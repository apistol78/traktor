/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_scene_ISceneEditorPlugin_H
#define traktor_scene_ISceneEditorPlugin_H

#include "Core/Object.h"
#include "Ui/Command.h"

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

class Widget;

		namespace custom
		{

class ToolBar;

		}
	}

	namespace scene
	{

class SceneEditorContext;

/*! \brief Scene editor plugin interface.
 */
class T_DLLCLASS ISceneEditorPlugin : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool create(ui::Widget* parent, ui::custom::ToolBar* toolBar) = 0;

	virtual bool handleCommand(const ui::Command& command) = 0;
};

	}
}

#endif	// traktor_scene_ISceneEditorPlugin_H
