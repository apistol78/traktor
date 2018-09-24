/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

class ColorControl;
class Slider;
class Static;
class ToolBarButton;
class ToolBarDropDown;
class ToolBarEmbed;
class ToolBarItemGroup;

	}

	namespace terrain
	{

class TerrainEditModifier;

class TerrainEditorPlugin : public scene::ISceneEditorPlugin
{
	T_RTTI_CLASS;

public:
	TerrainEditorPlugin(scene::SceneEditorContext* context);

	virtual bool create(ui::Widget* parent, ui::ToolBar* toolBar) T_OVERRIDE T_FINAL;

	virtual bool handleCommand(const ui::Command& command) T_OVERRIDE T_FINAL;

private:
	scene::SceneEditorContext* m_context;
	Ref< TerrainEditModifier > m_terrainEditModifier;
	Ref< ui::Widget > m_parent;
	Ref< ui::ToolBarButton > m_toolToggleEditTerrain;
	Ref< ui::ToolBarButton > m_toolToggleSplat;
	Ref< ui::ToolBarButton > m_toolToggleColor;
	Ref< ui::ToolBarButton > m_toolToggleEmissive;
	Ref< ui::ToolBarButton > m_toolToggleElevate;
	Ref< ui::ToolBarButton > m_toolToggleFlatten;
	Ref< ui::ToolBarButton > m_toolToggleAverage;
	Ref< ui::ToolBarButton > m_toolToggleSmooth;
	Ref< ui::ToolBarButton > m_toolToggleNoise;
	Ref< ui::ToolBarButton > m_toolToggleErode;
	Ref< ui::ToolBarButton > m_toolToggleCut;
	Ref< ui::ToolBarButton > m_toolToggleMaterial;
	Ref< ui::ToolBarButton > m_toolToggleFallOffSmooth;
	Ref< ui::ToolBarButton > m_toolToggleFallOffSharp;
	Ref< ui::ToolBarButton > m_toolToggleFallOffImage;
	Ref< ui::ToolBarButton > m_toolToggleSymmetryX;
	Ref< ui::ToolBarButton > m_toolToggleSymmetryZ;
	Ref< ui::Slider > m_sliderStrength;
	Ref< ui::Static > m_staticStrength;
	Ref< ui::ToolBarEmbed > m_toolStrength;
	Ref< ui::ColorControl > m_colorControl;
	Ref< ui::ToolBarEmbed > m_toolColor;
	Ref< ui::ToolBarDropDown > m_toolMaterial;
	Ref< ui::ToolBarDropDown > m_toolVisualize;
	Ref< ui::ToolBarItemGroup > m_toolGroup;

	void updateModifierState();

	void eventSliderStrengthChange(ui::ContentChangeEvent* event);

	void eventColorClick(ui::MouseButtonUpEvent* event);

	void eventModifierChanged(scene::ModifierChangedEvent* event);
};

	}
}

#endif	// traktor_terrain_TerrainEditorPlugin_H
