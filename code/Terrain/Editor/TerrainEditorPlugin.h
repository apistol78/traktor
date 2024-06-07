/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Scene/Editor/ISceneEditorPlugin.h"
#include "Ui/Events/AllEvents.h"

namespace traktor::scene
{

class ModifierChangedEvent;

}

namespace traktor::ui
{

class ColorControl;
class Slider;
class Static;
class ToolBarButton;
class ToolBarDropDown;
class ToolBarEmbed;
class ToolBarItemGroup;

}

namespace traktor::terrain
{

class TerrainEditModifier;

class TerrainEditorPlugin : public scene::ISceneEditorPlugin
{
	T_RTTI_CLASS;

public:
	explicit TerrainEditorPlugin(scene::SceneEditorContext* context);

	virtual bool create(ui::Widget* parent, ui::ToolBar* toolBar) override final;

	virtual bool handleCommand(const ui::Command& command) override final;

private:
	scene::SceneEditorContext* m_context;
	Ref< TerrainEditModifier > m_terrainEditModifier;
	Ref< ui::Widget > m_parent;
	Ref< ui::ToolBarButton > m_toolToggleEditTerrain;
	Ref< ui::ToolBarButton > m_toolToggleMaterial;
	Ref< ui::ToolBarButton > m_toolToggleColor;
	Ref< ui::ToolBarButton > m_toolToggleElevate;
	Ref< ui::ToolBarButton > m_toolToggleFlatten;
	Ref< ui::ToolBarButton > m_toolToggleSmooth;
	Ref< ui::ToolBarButton > m_toolToggleNoise;
	Ref< ui::ToolBarButton > m_toolToggleCut;
	Ref< ui::ToolBarButton > m_toolToggleAttribute;
	Ref< ui::ToolBarButton > m_toolToggleFallOffSmooth;
	Ref< ui::ToolBarButton > m_toolToggleFallOffSharp;
	Ref< ui::ToolBarButton > m_toolToggleFallOffImage;
	Ref< ui::Slider > m_sliderStrength;
	Ref< ui::ToolBarEmbed > m_toolStrength;
	Ref< ui::ColorControl > m_colorControl;
	Ref< ui::ToolBarEmbed > m_toolColor;
	Ref< ui::ToolBarDropDown > m_toolMaterial;
	Ref< ui::ToolBarDropDown > m_toolAttribute;
	Ref< ui::ToolBarDropDown > m_toolVisualize;
	Ref< ui::ToolBarItemGroup > m_toolGroup;

	void updateModifierState();

	void flattenUnderSpline();

	void eventSliderStrengthChange(ui::ContentChangeEvent* event);

	void eventColorClick(ui::MouseButtonUpEvent* event);

	void eventModifierChanged(scene::ModifierChangedEvent* event);
};

}
