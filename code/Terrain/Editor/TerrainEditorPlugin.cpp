/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Misc/String.h"
#include "Drawing/Image.h"
#include "I18N/Text.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/Events/ModifierChangedEvent.h"
#include "Terrain/TerrainComponent.h"
#include "Terrain/Editor/AverageBrush.h"
#include "Terrain/Editor/ColorBrush.h"
#include "Terrain/Editor/CutBrush.h"
#include "Terrain/Editor/ElevateBrush.h"
#include "Terrain/Editor/EmissiveBrush.h"
#include "Terrain/Editor/ErodeBrush.h"
#include "Terrain/Editor/FlattenBrush.h"
#include "Terrain/Editor/MaterialBrush.h"
#include "Terrain/Editor/NoiseBrush.h"
#include "Terrain/Editor/SmoothBrush.h"
#include "Terrain/Editor/SplatBrush.h"
#include "Terrain/Editor/TerrainEditorPlugin.h"
#include "Terrain/Editor/TerrainEditModifier.h"
#include "Ui/Application.h"
#include "Ui/Container.h"
#include "Ui/FileDialog.h"
#include "Ui/MenuItem.h"
#include "Ui/Slider.h"
#include "Ui/Static.h"
#include "Ui/StyleBitmap.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/ColorPicker/ColorControl.h"
#include "Ui/Custom/ColorPicker/ColorDialog.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarDropDown.h"
#include "Ui/Custom/ToolBar/ToolBarEmbed.h"
#include "Ui/Custom/ToolBar/ToolBarItemGroup.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainEditorPlugin", TerrainEditorPlugin, scene::ISceneEditorPlugin)

TerrainEditorPlugin::TerrainEditorPlugin(scene::SceneEditorContext* context)
:	m_context(context)
,	m_terrainEditModifier(new TerrainEditModifier(context))
{
}

bool TerrainEditorPlugin::create(ui::Widget* parent, ui::custom::ToolBar* toolBar)
{
	m_parent = parent;

	int32_t image = toolBar->addImage(new ui::StyleBitmap(L"Terrain.Terrain"), 15);

	m_toolToggleEditTerrain = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_EDIT_TERRAIN"), image + 6, ui::Command(L"Terrain.Editor.EditTerrain"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleSplat = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_SPLAT_BRUSH"), image + 9, ui::Command(L"Terrain.Editor.SplatBrush"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleColor = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_COLOR_BRUSH"), image + 8, ui::Command(L"Terrain.Editor.ColorBrush"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleEmissive = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_EMISSIVE_BRUSH"), image + 14, ui::Command(L"Terrain.Editor.EmissiveBrush"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleElevate = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_ELEVATE_BRUSH"), image + 0, ui::Command(L"Terrain.Editor.ElevateBrush"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleFlatten = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_FLATTEN_BRUSH"), image + 1, ui::Command(L"Terrain.Editor.FlattenBrush"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleAverage = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_AVERAGE_BRUSH"), image + 3, ui::Command(L"Terrain.Editor.AverageBrush"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleSmooth = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_SMOOTH_BRUSH"), image + 2, ui::Command(L"Terrain.Editor.SmoothBrush"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleNoise = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_NOISE_BRUSH"), image + 10, ui::Command(L"Terrain.Editor.NoiseBrush"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleErode = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_ERODE_BRUSH"), image + 13, ui::Command(L"Terrain.Editor.ErodeBrush"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleCut = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_CUT_BRUSH"), image + 7, ui::Command(L"Terrain.Editor.CutBrush"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleMaterial = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_MATERIAL_BRUSH"), image + 9, ui::Command(L"Terrain.Editor.MaterialBrush"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleFallOffSmooth = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_SMOOTH_FALLOFF"), image + 4, ui::Command(L"Terrain.Editor.SmoothFallOff"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleFallOffSharp = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_SHARP_FALLOFF"), image + 5, ui::Command(L"Terrain.Editor.SharpFallOff"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleFallOffImage = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_SHARP_IMAGE"), image + 5, ui::Command(L"Terrain.Editor.ImageFallOff"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleSymmetryX = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_SYMMETRY_X"), image + 11, ui::Command(L"Terrain.Editor.SymmetryX"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleSymmetryZ = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_SYMMETRY_Z"), image + 12, ui::Command(L"Terrain.Editor.SymmetryZ"), ui::custom::ToolBarButton::BsDefaultToggle);

	Ref< ui::Container > containerStrength = new ui::Container();
	containerStrength->create(toolBar, ui::WsNone, new ui::TableLayout(L"100,35", L"24", 2, 2));

	m_sliderStrength = new ui::Slider();
	m_sliderStrength->create(containerStrength);
	m_sliderStrength->setRange(1, 10);
	m_sliderStrength->setValue(5);
	m_sliderStrength->addEventHandler< ui::ContentChangeEvent >(this, &TerrainEditorPlugin::eventSliderStrengthChange);

	m_staticStrength = new ui::Static();
	m_staticStrength->create(containerStrength, L"50%");

	m_toolStrength = new ui::custom::ToolBarEmbed(containerStrength, ui::dpi96(135));

	m_colorControl = new ui::custom::ColorControl();
	m_colorControl->create(toolBar, ui::WsBorder);
	m_colorControl->setColor(Color4ub(128, 128, 128, 255));
	m_colorControl->addEventHandler< ui::MouseButtonUpEvent >(this, &TerrainEditorPlugin::eventColorClick);

	m_toolColor = new ui::custom::ToolBarEmbed(m_colorControl, 32);

	m_toolMaterial = new ui::custom::ToolBarDropDown(ui::Command(L"Terrain.Editor.SelectMaterial"), ui::dpi96(80), i18n::Text(L"TERRAIN_EDITOR_MATERIAL"));
	m_toolMaterial->add(L"Material 1");
	m_toolMaterial->add(L"Material 2");
	m_toolMaterial->add(L"Material 3");
	m_toolMaterial->add(L"Material 4");
	m_toolMaterial->select(0);

	m_toolVisualize = new ui::custom::ToolBarDropDown(ui::Command(L"Terrain.Editor.SelectVisualize"), ui::dpi96(100), i18n::Text(L"TERRAIN_EDITOR_VISUALIZE"));
	m_toolVisualize->add(L"Default");
	m_toolVisualize->add(L"Surface LOD");
	m_toolVisualize->add(L"Patch LOD");
	m_toolVisualize->add(L"Color Map");
	m_toolVisualize->add(L"Normal Map");
	m_toolVisualize->add(L"Height Map");
	m_toolVisualize->add(L"Splat Map");
	m_toolVisualize->add(L"Cut Map");
	m_toolVisualize->add(L"Material Map");
	m_toolVisualize->select(0);
	
	m_toolToggleElevate->setToggled(true);
	m_toolToggleFallOffSmooth->setToggled(true);

	// Create a tool group containing all toolbar items which should
	// have enable/disable automatically when terrain edit modifier is active.
	m_toolGroup = new ui::custom::ToolBarItemGroup();

	toolBar->addItem(new ui::custom::ToolBarSeparator());
	toolBar->addItem(m_toolToggleEditTerrain);
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleSplat));
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleColor));
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleEmissive));
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleElevate));
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleFlatten));
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleAverage));
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleSmooth));
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleNoise));
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleErode));
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleCut));
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleMaterial));
	toolBar->addItem(new ui::custom::ToolBarSeparator());
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleFallOffSmooth));
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleFallOffSharp));
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleFallOffImage));
	toolBar->addItem(new ui::custom::ToolBarSeparator());
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleSymmetryX));
	toolBar->addItem(m_toolGroup->addItem(m_toolToggleSymmetryZ));
	toolBar->addItem(new ui::custom::ToolBarSeparator());
	toolBar->addItem(m_toolGroup->addItem(m_toolStrength));
	toolBar->addItem(m_toolGroup->addItem(m_toolColor));
	toolBar->addItem(m_toolGroup->addItem(m_toolMaterial));
	toolBar->addItem(new ui::custom::ToolBarSeparator());
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
		ui::custom::ToolBarButton* toolSelected = 0;

		if (command == L"Terrain.Editor.SplatBrush")
			toolSelected = m_toolToggleSplat;
		else if (command == L"Terrain.Editor.ColorBrush")
			toolSelected = m_toolToggleColor;
		else if (command == L"Terrain.Editor.EmissiveBrush")
			toolSelected = m_toolToggleEmissive;
		else if (command == L"Terrain.Editor.ElevateBrush")
			toolSelected = m_toolToggleElevate;
		else if (command == L"Terrain.Editor.FlattenBrush")
			toolSelected = m_toolToggleFlatten;
		else if (command == L"Terrain.Editor.AverageBrush")
			toolSelected = m_toolToggleAverage;
		else if (command == L"Terrain.Editor.SmoothBrush")
			toolSelected = m_toolToggleSmooth;
		else if (command == L"Terrain.Editor.NoiseBrush")
			toolSelected = m_toolToggleNoise;
		else if (command == L"Terrain.Editor.ErodeBrush")
			toolSelected = m_toolToggleErode;
		else if (command == L"Terrain.Editor.CutBrush")
			toolSelected = m_toolToggleCut;
		else if (command == L"Terrain.Editor.MaterialBrush")
			toolSelected = m_toolToggleMaterial;

		if (toolSelected)
		{
			m_toolToggleSplat->setToggled(m_toolToggleSplat == toolSelected);
			m_toolToggleColor->setToggled(m_toolToggleColor == toolSelected);
			m_toolToggleEmissive->setToggled(m_toolToggleEmissive == toolSelected);
			m_toolToggleElevate->setToggled(m_toolToggleElevate == toolSelected);
			m_toolToggleFlatten->setToggled(m_toolToggleFlatten == toolSelected);
			m_toolToggleAverage->setToggled(m_toolToggleAverage == toolSelected);
			m_toolToggleSmooth->setToggled(m_toolToggleSmooth == toolSelected);
			m_toolToggleNoise->setToggled(m_toolToggleNoise == toolSelected);
			m_toolToggleErode->setToggled(m_toolToggleErode == toolSelected);
			m_toolToggleCut->setToggled(m_toolToggleCut == toolSelected);
			m_toolToggleMaterial->setToggled(m_toolToggleMaterial == toolSelected);
			updateModifierState();
			return true;
		}
	}

	{
		ui::custom::ToolBarButton* toolSelected = 0;

		if (command == L"Terrain.Editor.SmoothFallOff")
			toolSelected = m_toolToggleFallOffSmooth;
		else if (command == L"Terrain.Editor.SharpFallOff")
			toolSelected = m_toolToggleFallOffSharp;
		else if (command == L"Terrain.Editor.ImageFallOff")
		{
			ui::FileDialog fileDialog;

			if (!fileDialog.create(m_parent, i18n::Text(L"TERRAIN_EDITOR_BROWSE_IMAGE"), L"All files (*.*);*.*"))
				return false;

			Path path;
			if (fileDialog.showModal(path) == ui::DrOk)
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
		ui::custom::ToolBarButton* toolSelected = 0;

		if (command == L"Terrain.Editor.SymmetryX")
			toolSelected = m_toolToggleSymmetryX;
		else if (command == L"Terrain.Editor.SymmetryZ")
			toolSelected = m_toolToggleSymmetryZ;

		if (toolSelected)
		{
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
		if (command == L"Terrain.Editor.SelectVisualize")
		{
			updateModifierState();
			return true;
		}
	}

	return false;
}

void TerrainEditorPlugin::updateModifierState()
{
	if (m_toolToggleSplat->isToggled())
		m_terrainEditModifier->setBrush(type_of< SplatBrush >());
	else if (m_toolToggleColor->isToggled())
		m_terrainEditModifier->setBrush(type_of< ColorBrush >());
	else if (m_toolToggleEmissive->isToggled())
		m_terrainEditModifier->setBrush(type_of< EmissiveBrush >());
	else if (m_toolToggleElevate->isToggled())
		m_terrainEditModifier->setBrush(type_of< ElevateBrush >());
	else if (m_toolToggleFlatten->isToggled())
		m_terrainEditModifier->setBrush(type_of< FlattenBrush >());
	else if (m_toolToggleAverage->isToggled())
		m_terrainEditModifier->setBrush(type_of< AverageBrush >());
	else if (m_toolToggleSmooth->isToggled())
		m_terrainEditModifier->setBrush(type_of< SmoothBrush >());
	else if (m_toolToggleNoise->isToggled())
		m_terrainEditModifier->setBrush(type_of< NoiseBrush >());
	else if (m_toolToggleErode->isToggled())
		m_terrainEditModifier->setBrush(type_of< ErodeBrush >());
	else if (m_toolToggleCut->isToggled())
		m_terrainEditModifier->setBrush(type_of< CutBrush >());
	else if (m_toolToggleMaterial->isToggled())
		m_terrainEditModifier->setBrush(type_of< MaterialBrush >());

	if (m_toolToggleFallOffSmooth->isToggled())
		m_terrainEditModifier->setFallOff(L"Terrain.Editor.SmoothFallOff");
	else if (m_toolToggleFallOffSharp->isToggled())
		m_terrainEditModifier->setFallOff(L"Terrain.Editor.SharpFallOff");
	else if (m_toolToggleFallOffImage->isToggled())
		m_terrainEditModifier->setFallOff(L"Terrain.Editor.ImageFallOff");

	uint32_t symmetry = 0;
	if (m_toolToggleSymmetryX->isToggled())
		symmetry |= 1;
	if (m_toolToggleSymmetryZ->isToggled())
		symmetry |= 2;
	m_terrainEditModifier->setSymmetry(symmetry);

	int32_t material = m_toolMaterial->getSelected();
	if (material >= 0)
		m_terrainEditModifier->setMaterial(material);

	int32_t visualize = m_toolVisualize->getSelected();
	if (visualize >= 0)
		m_terrainEditModifier->setVisualizeMode((TerrainComponent::VisualizeMode)visualize);

	m_terrainEditModifier->setStrength(m_sliderStrength->getValue() / 10.0f);
}

void TerrainEditorPlugin::eventSliderStrengthChange(ui::ContentChangeEvent* event)
{
	m_terrainEditModifier->setStrength(m_sliderStrength->getValue() / 10.0f);
	m_staticStrength->setText(toString(int32_t(m_sliderStrength->getValue() * 10)) + L"%");
}

void TerrainEditorPlugin::eventColorClick(ui::MouseButtonUpEvent* event)
{
	ui::custom::ColorDialog colorDialog;
	colorDialog.create(
		m_parent,
		i18n::Text(L"COLOR_DIALOG_TEXT"),
		ui::custom::ColorDialog::WsDefaultFixed,
		m_colorControl->getColor()
	);
	if (colorDialog.showModal() == ui::DrOk)
	{
		Color4ub color = colorDialog.getColor();
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
}
