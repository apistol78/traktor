/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/String.h"
#include "Drawing/Image.h"
#include "I18N/Text.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/Events/ModifierChangedEvent.h"
#include "Terrain/TerrainComponent.h"
#include "Terrain/Editor/ColorBrush.h"
#include "Terrain/Editor/CutBrush.h"
#include "Terrain/Editor/ElevateBrush.h"
#include "Terrain/Editor/FlattenBrush.h"
#include "Terrain/Editor/AttributeBrush.h"
#include "Terrain/Editor/MaterialBrush.h"
#include "Terrain/Editor/NoiseBrush.h"
#include "Terrain/Editor/SmoothBrush.h"
#include "Terrain/Editor/TerrainEditorPlugin.h"
#include "Terrain/Editor/TerrainEditModifier.h"
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/MenuItem.h"
#include "Ui/Slider.h"
#include "Ui/Static.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/FileDialog.h"
#include "Ui/ColorPicker/ColorControl.h"
#include "Ui/ColorPicker/ColorDialog.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButton.h"
#include "Ui/ToolBar/ToolBarDropDown.h"
#include "Ui/ToolBar/ToolBarEmbed.h"
#include "Ui/ToolBar/ToolBarItemGroup.h"
#include "Ui/ToolBar/ToolBarSeparator.h"

namespace traktor::terrain
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainEditorPlugin", TerrainEditorPlugin, scene::ISceneEditorPlugin)

TerrainEditorPlugin::TerrainEditorPlugin(scene::SceneEditorContext* context)
:	m_context(context)
,	m_terrainEditModifier(new TerrainEditModifier(context))
{
}

bool TerrainEditorPlugin::create(ui::Widget* parent, ui::ToolBar* toolBar)
{
	m_parent = parent;

	const int32_t image = toolBar->addImage(new ui::StyleBitmap(L"Terrain.Terrain", 0));
	for (int32_t i = 1; i < 17; ++i)
		toolBar->addImage(new ui::StyleBitmap(L"Terrain.Terrain", i));

	m_toolToggleEditTerrain = new ui::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_EDIT_TERRAIN"), image + 6, ui::Command(L"Terrain.Editor.EditTerrain"), ui::ToolBarButton::BsDefaultToggle);
	m_toolToggleMaterial = new ui::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_MATERIAL_BRUSH"), image + 16, ui::Command(L"Terrain.Editor.MaterialBrush"), ui::ToolBarButton::BsDefaultToggle);
	m_toolToggleColor = new ui::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_COLOR_BRUSH"), image + 8, ui::Command(L"Terrain.Editor.ColorBrush"), ui::ToolBarButton::BsDefaultToggle);
	m_toolToggleElevate = new ui::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_ELEVATE_BRUSH"), image + 0, ui::Command(L"Terrain.Editor.ElevateBrush"), ui::ToolBarButton::BsDefaultToggle);
	m_toolToggleFlatten = new ui::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_FLATTEN_BRUSH"), image + 1, ui::Command(L"Terrain.Editor.FlattenBrush"), ui::ToolBarButton::BsDefaultToggle);
	m_toolToggleSmooth = new ui::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_SMOOTH_BRUSH"), image + 2, ui::Command(L"Terrain.Editor.SmoothBrush"), ui::ToolBarButton::BsDefaultToggle);
	m_toolToggleNoise = new ui::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_NOISE_BRUSH"), image + 10, ui::Command(L"Terrain.Editor.NoiseBrush"), ui::ToolBarButton::BsDefaultToggle);
	m_toolToggleCut = new ui::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_CUT_BRUSH"), image + 7, ui::Command(L"Terrain.Editor.CutBrush"), ui::ToolBarButton::BsDefaultToggle);
	m_toolToggleAttribute = new ui::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_ATTRIBUTE_BRUSH"), image + 9, ui::Command(L"Terrain.Editor.AttributeBrush"), ui::ToolBarButton::BsDefaultToggle);
	m_toolToggleFallOffSmooth = new ui::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_SMOOTH_FALLOFF"), image + 4, ui::Command(L"Terrain.Editor.SmoothFallOff"), ui::ToolBarButton::BsDefaultToggle);
	m_toolToggleFallOffSharp = new ui::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_SHARP_FALLOFF"), image + 5, ui::Command(L"Terrain.Editor.SharpFallOff"), ui::ToolBarButton::BsDefaultToggle);
	m_toolToggleFallOffImage = new ui::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_SHARP_IMAGE"), image + 15, ui::Command(L"Terrain.Editor.ImageFallOff"), ui::ToolBarButton::BsDefaultToggle);

	m_sliderStrength = new ui::Slider();
	m_sliderStrength->create(toolBar);
	m_sliderStrength->setRange(0, 10);
	m_sliderStrength->setValue(5);
	m_sliderStrength->addEventHandler< ui::ContentChangeEvent >(this, &TerrainEditorPlugin::eventSliderStrengthChange);

	m_toolStrength = new ui::ToolBarEmbed(m_sliderStrength, 100_ut);

	m_colorControl = new ui::ColorControl();
	m_colorControl->create(toolBar, ui::WsBorder);
	m_colorControl->setColor(Color4ub(128, 128, 128, 255));
	m_colorControl->addEventHandler< ui::MouseButtonUpEvent >(this, &TerrainEditorPlugin::eventColorClick);

	m_toolColor = new ui::ToolBarEmbed(m_colorControl, 24_ut, 16_ut);

	m_toolMaterial = new ui::ToolBarDropDown(ui::Command(L"Terrain.Editor.SelectMaterial"), 80_ut, i18n::Text(L"TERRAIN_EDITOR_MATERIAL"));
	m_toolMaterial->add(L"1");
	m_toolMaterial->add(L"2");
	m_toolMaterial->add(L"3");
	m_toolMaterial->add(L"4");
	m_toolMaterial->select(0);

	m_toolAttribute = new ui::ToolBarDropDown(ui::Command(L"Terrain.Editor.SelectAttribute"), 80_ut, i18n::Text(L"TERRAIN_EDITOR_ATTRIBUTE"));
	m_toolAttribute->add(L"1");
	m_toolAttribute->add(L"2");
	m_toolAttribute->add(L"3");
	m_toolAttribute->add(L"4");
	m_toolAttribute->select(0);

	m_toolVisualize = new ui::ToolBarDropDown(ui::Command(L"Terrain.Editor.SelectVisualize"), 100_ut, i18n::Text(L"TERRAIN_EDITOR_VISUALIZE"));
	m_toolVisualize->add(L"Default");
	m_toolVisualize->add(L"Surface LOD");
	m_toolVisualize->add(L"Patch LOD");
	m_toolVisualize->add(L"Color Map");
	m_toolVisualize->add(L"Normal Map");
	m_toolVisualize->add(L"Height Map");
	m_toolVisualize->add(L"Splat Map");
	m_toolVisualize->add(L"Cut Map");
	m_toolVisualize->select(0);

	m_toolToggleElevate->setToggled(true);
	m_toolToggleFallOffSmooth->setToggled(true);

	// Create a tool group containing all toolbar items which should
	// have enable/disable automatically when terrain edit modifier is active.
	m_toolGroup = new ui::ToolBarItemGroup();

	toolBar->addItem(new ui::ToolBarSeparator());
	toolBar->addItem(m_toolToggleEditTerrain);
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleMaterial));
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleColor));
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleElevate));
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleFlatten));
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleSmooth));
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleNoise));
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleCut));
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleAttribute));
	toolBar->addItem(new ui::ToolBarSeparator());
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleFallOffSmooth));
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleFallOffSharp));
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleFallOffImage));
	toolBar->addItem(new ui::ToolBarSeparator());
	toolBar->addItem(m_toolGroup->addItem(m_toolStrength));
	toolBar->addItem(m_toolGroup->addItem(m_toolColor));
	toolBar->addItem(m_toolGroup->addItem(m_toolMaterial));
	toolBar->addItem(m_toolGroup->addItem(m_toolAttribute));
	toolBar->addItem(new ui::ToolBarSeparator());
	toolBar->addItem(m_toolGroup->addItem(new ui::ToolBarButton(L"Flatten", ui::Command(L"Terrain.Editor.FlattenUnderSpline"))));
	toolBar->addItem(new ui::ToolBarSeparator());
	toolBar->addItem(m_toolGroup->addItem(m_toolVisualize));

	// Group is initially disabled as terrain edit modifier hasn't yet been selected.
	m_toolGroup->setEnable(false);

	updateModifierState();

	m_context->addEventHandler< scene::ModifierChangedEvent >(this, &TerrainEditorPlugin::eventModifierChanged);
	return true;
}

bool TerrainEditorPlugin::handleCommand(const ui::Command& command)
{
	{
		if (command == L"Terrain.Editor.EditTerrain")
		{
			m_context->setPlaying(false);
			m_context->setModifier(m_terrainEditModifier);
			return true;
		}
	}

	{
		ui::ToolBarButton* toolSelected = nullptr;

		if (command == L"Terrain.Editor.MaterialBrush")
			toolSelected = m_toolToggleMaterial;
		else if (command == L"Terrain.Editor.ColorBrush")
			toolSelected = m_toolToggleColor;
		else if (command == L"Terrain.Editor.ElevateBrush")
			toolSelected = m_toolToggleElevate;
		else if (command == L"Terrain.Editor.FlattenBrush")
			toolSelected = m_toolToggleFlatten;
		else if (command == L"Terrain.Editor.SmoothBrush")
			toolSelected = m_toolToggleSmooth;
		else if (command == L"Terrain.Editor.NoiseBrush")
			toolSelected = m_toolToggleNoise;
		else if (command == L"Terrain.Editor.CutBrush")
			toolSelected = m_toolToggleCut;
		else if (command == L"Terrain.Editor.AttributeBrush")
			toolSelected = m_toolToggleAttribute;

		if (toolSelected)
		{
			m_toolToggleMaterial->setToggled(m_toolToggleMaterial == toolSelected);
			m_toolToggleColor->setToggled(m_toolToggleColor == toolSelected);
			m_toolToggleElevate->setToggled(m_toolToggleElevate == toolSelected);
			m_toolToggleFlatten->setToggled(m_toolToggleFlatten == toolSelected);
			m_toolToggleSmooth->setToggled(m_toolToggleSmooth == toolSelected);
			m_toolToggleNoise->setToggled(m_toolToggleNoise == toolSelected);
			m_toolToggleCut->setToggled(m_toolToggleCut == toolSelected);
			m_toolToggleAttribute->setToggled(m_toolToggleAttribute == toolSelected);
			updateModifierState();
			return true;
		}
	}

	{
		ui::ToolBarButton* toolSelected = nullptr;

		if (command == L"Terrain.Editor.SmoothFallOff")
			toolSelected = m_toolToggleFallOffSmooth;
		else if (command == L"Terrain.Editor.SharpFallOff")
			toolSelected = m_toolToggleFallOffSharp;
		else if (command == L"Terrain.Editor.ImageFallOff")
		{
			ui::FileDialog fileDialog;

			if (!fileDialog.create(m_parent, type_name(this), i18n::Text(L"TERRAIN_EDITOR_BROWSE_IMAGE"), L"All files (*.*);*.*"))
				return false;

			Path path;
			if (fileDialog.showModal(path) == ui::DialogResult::Ok)
			{
				Ref< drawing::Image > image = drawing::Image::load(path);
				if (image)
				{
					m_terrainEditModifier->setFallOffImage(image);
					toolSelected = m_toolToggleFallOffImage;
				}
			}

			fileDialog.destroy();
		}

		if (toolSelected)
		{
			m_toolToggleFallOffSmooth->setToggled(m_toolToggleFallOffSmooth == toolSelected);
			m_toolToggleFallOffSharp->setToggled(m_toolToggleFallOffSharp == toolSelected);
			m_toolToggleFallOffImage->setToggled(m_toolToggleFallOffImage == toolSelected);
			updateModifierState();
			return true;
		}
	}

	{
		if (command == L"Terrain.Editor.SelectMaterial")
		{
			updateModifierState();
			return true;
		}
	}

	{
		if (command == L"Terrain.Editor.SelectAttribute")
		{
			updateModifierState();
			return true;
		}
	}

	{
		if (command == L"Terrain.Editor.SelectVisualize")
		{
			updateModifierState();
			return true;
		}
	}

	{
		if (m_terrainEditModifier->handleCommand(command))
			return true;
	}

	return false;
}

void TerrainEditorPlugin::updateModifierState()
{
	if (m_toolToggleMaterial->isToggled())
		m_terrainEditModifier->setBrush(type_of< MaterialBrush >());
	else if (m_toolToggleColor->isToggled())
		m_terrainEditModifier->setBrush(type_of< ColorBrush >());
	else if (m_toolToggleElevate->isToggled())
		m_terrainEditModifier->setBrush(type_of< ElevateBrush >());
	else if (m_toolToggleFlatten->isToggled())
		m_terrainEditModifier->setBrush(type_of< FlattenBrush >());
	else if (m_toolToggleSmooth->isToggled())
		m_terrainEditModifier->setBrush(type_of< SmoothBrush >());
	else if (m_toolToggleNoise->isToggled())
		m_terrainEditModifier->setBrush(type_of< NoiseBrush >());
	else if (m_toolToggleCut->isToggled())
		m_terrainEditModifier->setBrush(type_of< CutBrush >());
	else if (m_toolToggleAttribute->isToggled())
		m_terrainEditModifier->setBrush(type_of< AttributeBrush >());

	if (m_toolToggleFallOffSmooth->isToggled())
		m_terrainEditModifier->setFallOff(L"Terrain.Editor.SmoothFallOff");
	else if (m_toolToggleFallOffSharp->isToggled())
		m_terrainEditModifier->setFallOff(L"Terrain.Editor.SharpFallOff");
	else if (m_toolToggleFallOffImage->isToggled())
		m_terrainEditModifier->setFallOff(L"Terrain.Editor.ImageFallOff");

	const int32_t material = m_toolMaterial->getSelected();
	if (material >= 0)
		m_terrainEditModifier->setMaterial(material);

	const int32_t attribute = m_toolAttribute->getSelected();
	if (attribute >= 0)
		m_terrainEditModifier->setAttribute(attribute);

	const int32_t visualize = m_toolVisualize->getSelected();
	if (visualize >= 0)
		m_terrainEditModifier->setVisualizeMode((TerrainComponent::VisualizeMode)visualize);

	m_terrainEditModifier->setStrength(m_sliderStrength->getValue() / 10.0f);
}

void TerrainEditorPlugin::eventSliderStrengthChange(ui::ContentChangeEvent* event)
{
	m_terrainEditModifier->setStrength(m_sliderStrength->getValue() / 10.0f);
}

void TerrainEditorPlugin::eventColorClick(ui::MouseButtonUpEvent* event)
{
	ui::ColorDialog colorDialog;
	colorDialog.create(
		m_parent,
		i18n::Text(L"COLOR_DIALOG_TEXT"),
		ui::ColorDialog::WsDefaultFixed,
		Color4f::fromColor4ub(m_colorControl->getColor())
	);
	if (colorDialog.showModal() == ui::DialogResult::Ok)
	{
		Color4ub color = colorDialog.getColor().toColor4ub();
		m_colorControl->setColor(color);
		m_terrainEditModifier->setColor(Color4f(
			color.r / 255.0f,
			color.g / 255.0f,
			color.b / 255.0f,
			1.0f
		));
	}
	colorDialog.destroy();
}

void TerrainEditorPlugin::eventModifierChanged(scene::ModifierChangedEvent* event)
{
	m_toolToggleEditTerrain->setToggled(m_context->getModifier() == m_terrainEditModifier);
	m_toolGroup->setEnable(m_context->getModifier() == m_terrainEditModifier);
}

}
