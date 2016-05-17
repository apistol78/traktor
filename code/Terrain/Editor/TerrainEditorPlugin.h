#ifndef traktor_terrain_TerrainEditorPlugin_H
#define traktor_terrain_TerrainEditorPlugin_H

#include "Scene/Editor/ISceneEditorPlugin.h"
#include "Ui/Events/AllEvents.h"

namespace traktor
{
	namespace scene
	{

class ModifierChangedEvent;

	}

	namespace ui
	{

class Slider;
class Static;

		namespace custom
		{

class ColorControl;
class ToolBarButton;
class ToolBarDropDown;
class ToolBarEmbed;
class ToolBarItemGroup;

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

	virtual bool create(ui::Widget* parent, ui::custom::ToolBar* toolBar) T_OVERRIDE T_FINAL;

	virtual bool handleCommand(const ui::Command& command) T_OVERRIDE T_FINAL;

private:
	scene::SceneEditorContext* m_context;
	Ref< TerrainEditModifier > m_terrainEditModifier;
	Ref< ui::Widget > m_parent;
	Ref< ui::custom::ToolBarButton > m_toolToggleEditTerrain;
	Ref< ui::custom::ToolBarButton > m_toolToggleSplat;
	Ref< ui::custom::ToolBarButton > m_toolToggleColor;
	Ref< ui::custom::ToolBarButton > m_toolToggleEmissive;
	Ref< ui::custom::ToolBarButton > m_toolToggleElevate;
	Ref< ui::custom::ToolBarButton > m_toolToggleFlatten;
	Ref< ui::custom::ToolBarButton > m_toolToggleAverage;
	Ref< ui::custom::ToolBarButton > m_toolToggleSmooth;
	Ref< ui::custom::ToolBarButton > m_toolToggleNoise;
	Ref< ui::custom::ToolBarButton > m_toolToggleErode;
	Ref< ui::custom::ToolBarButton > m_toolToggleCut;
	Ref< ui::custom::ToolBarButton > m_toolToggleMaterial;
	Ref< ui::custom::ToolBarButton > m_toolToggleFallOffSmooth;
	Ref< ui::custom::ToolBarButton > m_toolToggleFallOffSharp;
	Ref< ui::custom::ToolBarButton > m_toolToggleFallOffImage;
	Ref< ui::custom::ToolBarButton > m_toolToggleSymmetryX;
	Ref< ui::custom::ToolBarButton > m_toolToggleSymmetryZ;
	Ref< ui::Slider > m_sliderStrength;
	Ref< ui::Static > m_staticStrength;
	Ref< ui::custom::ToolBarEmbed > m_toolStrength;
	Ref< ui::custom::ColorControl > m_colorControl;
	Ref< ui::custom::ToolBarEmbed > m_toolColor;
	Ref< ui::custom::ToolBarDropDown > m_toolMaterial;
	Ref< ui::custom::ToolBarDropDown > m_toolVisualize;
	Ref< ui::custom::ToolBarItemGroup > m_toolGroup;

	void updateModifierState();

	void eventSliderStrengthChange(ui::ContentChangeEvent* event);

	void eventColorClick(ui::MouseButtonUpEvent* event);

	void eventModifierChanged(scene::ModifierChangedEvent* event);
};

	}
}

#endif	// traktor_terrain_TerrainEditorPlugin_H
