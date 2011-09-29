#ifndef traktor_terrain_TerrainEditorPlugin_H
#define traktor_terrain_TerrainEditorPlugin_H

#include <map>
#include "Core/Guid.h"
#include "Scene/Editor/ISceneEditorPlugin.h"

namespace traktor
{
	namespace terrain
	{

class HeightfieldCompositor;

class TerrainEditorPlugin : public scene::ISceneEditorPlugin
{
	T_RTTI_CLASS;

public:
	TerrainEditorPlugin(scene::SceneEditorContext* context);

	virtual bool create(ui::Widget* parent, ui::custom::ToolBar* toolBar);

	virtual bool handleCommand(const ui::Command& command);

	HeightfieldCompositor* getCompositor(const Guid& assetGuid);

	bool saveCompositors();

private:
	scene::SceneEditorContext* m_context;
	std::map< Guid, Ref< HeightfieldCompositor > > m_compositors;
};

	}
}

#endif	// traktor_terrain_TerrainEditorPlugin_H
