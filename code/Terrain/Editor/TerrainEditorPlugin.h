#ifndef traktor_terrain_HeightfieldEditorPlugin_H
#define traktor_terrain_HeightfieldEditorPlugin_H

#include <map>
#include "Core/Guid.h"
#include "Scene/Editor/ISceneEditorPlugin.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TERRAIN_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

class ToolBarButton;

		}
	}

	namespace terrain
	{

class T_DLLCLASS TerrainEditorPlugin : public scene::ISceneEditorPlugin
{
	T_RTTI_CLASS;

public:
	TerrainEditorPlugin(scene::SceneEditorContext* context);

	virtual bool create(ui::Widget* parent, ui::custom::ToolBar* toolBar);

	virtual bool handleCommand(const ui::Command& command);

	int32_t getSelectedTool() const { return m_selectedTool; }

private:
	scene::SceneEditorContext* m_context;
	Ref< ui::custom::ToolBarButton > m_toolRaiseTool;
	Ref< ui::custom::ToolBarButton > m_toolFlattenTool;
	Ref< ui::custom::ToolBarButton > m_toolSmoothTool;
	int32_t m_selectedTool;
};

	}
}

#endif	// traktor_terrain_HeightfieldEditorPlugin_H
