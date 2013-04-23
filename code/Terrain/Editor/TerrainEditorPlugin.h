#ifndef traktor_terrain_TerrainEditorPlugin_H
#define traktor_terrain_TerrainEditorPlugin_H

#include "Scene/Editor/ISceneEditorPlugin.h"

namespace traktor
{
	namespace ui
	{

class Event;
class Slider;
class Static;

		namespace custom
		{

class ColorControl;
class ToolBarButton;
class ToolBarEmbed;

		}
	}

	namespace terrain
	{

class TerrainEditModifier;

class TerrainEditorPlugin : public scene::ISceneEditorPlugin
{
	T_RTTI_CLASS;

public:
	TerrainEditorPlugin(scene::SceneEditorContext* context);

	virtual bool create(ui::Widget* parent, ui::custom::ToolBar* toolBar);

	virtual bool handleCommand(const ui::Command& command);

private:
	scene::SceneEditorContext* m_context;
	Ref< TerrainEditModifier > m_terrainEditModifier;
	Ref< ui::Widget > m_parent;
	Ref< ui::custom::ToolBarButton > m_toolToggleEditTerrain;
	Ref< ui::custom::ToolBarButton > m_toolToggleColor;
	Ref< ui::custom::ToolBarButton > m_toolToggleElevate;
	Ref< ui::custom::ToolBarButton > m_toolToggleFlatten;
	Ref< ui::custom::ToolBarButton > m_toolToggleAverage;
	Ref< ui::custom::ToolBarButton > m_toolToggleSmooth;
	Ref< ui::custom::ToolBarButton > m_toolToggleCut;
	Ref< ui::custom::ToolBarButton > m_toolToggleFallOffSmooth;
	Ref< ui::custom::ToolBarButton > m_toolToggleFallOffSharp;
	Ref< ui::Slider > m_sliderStrength;
	Ref< ui::Static > m_staticStrength;
	Ref< ui::custom::ToolBarEmbed > m_toolStrength;
	Ref< ui::custom::ColorControl > m_colorControl;
	Ref< ui::custom::ToolBarEmbed > m_toolColor;

	void eventSliderStrengthChange(ui::Event* event);

	void eventColorClick(ui::Event* event);

	void eventModifierChanged(ui::Event* event);
};

	}
}

#endif	// traktor_terrain_TerrainEditorPlugin_H
