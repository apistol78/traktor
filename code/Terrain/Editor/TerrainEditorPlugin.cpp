#include "Core/Misc/String.h"
#include "I18N/Text.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Terrain/Editor/TerrainEditorPlugin.h"
#include "Terrain/Editor/TerrainEditModifier.h"
#include "Ui/Bitmap.h"
#include "Ui/Container.h"
#include "Ui/MenuItem.h"
#include "Ui/MethodHandler.h"
#include "Ui/Slider.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/ColorPicker/ColorControl.h"
#include "Ui/Custom/ColorPicker/ColorDialog.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarDropDown.h"
#include "Ui/Custom/ToolBar/ToolBarEmbed.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"

// Resources
#include "Resources/Terrain.h"

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

	int32_t image = toolBar->addImage(ui::Bitmap::load(c_ResourceTerrain, sizeof(c_ResourceTerrain), L"png"), 13);

	m_toolToggleEditTerrain = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_EDIT_TERRAIN"), image + 6, ui::Command(L"Terrain.Editor.EditTerrain"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleMaterial = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_MATERIAL_BRUSH"), image + 9, ui::Command(L"Terrain.Editor.MaterialBrush"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleColor = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_COLOR_BRUSH"), image + 8, ui::Command(L"Terrain.Editor.ColorBrush"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleElevate = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_ELEVATE_BRUSH"), image + 0, ui::Command(L"Terrain.Editor.ElevateBrush"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleFlatten = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_FLATTEN_BRUSH"), image + 1, ui::Command(L"Terrain.Editor.FlattenBrush"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleAverage = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_AVERAGE_BRUSH"), image + 3, ui::Command(L"Terrain.Editor.AverageBrush"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleSmooth = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_SMOOTH_BRUSH"), image + 2, ui::Command(L"Terrain.Editor.SmoothBrush"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleNoise = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_NOISE_BRUSH"), image + 10, ui::Command(L"Terrain.Editor.NoiseBrush"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleCut = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_CUT_BRUSH"), image + 7, ui::Command(L"Terrain.Editor.CutBrush"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleFallOffSmooth = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_SMOOTH_FALLOFF"), image + 4, ui::Command(L"Terrain.Editor.SmoothFallOff"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleFallOffSharp = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_SHARP_FALLOFF"), image + 5, ui::Command(L"Terrain.Editor.SharpFallOff"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleSymmetryX = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_SYMMETRY_X"), image + 11, ui::Command(L"Terrain.Editor.SymmetryX"), ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleSymmetryZ = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_SYMMETRY_Z"), image + 12, ui::Command(L"Terrain.Editor.SymmetryZ"), ui::custom::ToolBarButton::BsDefaultToggle);

	Ref< ui::Container > containerStrength = new ui::Container();
	containerStrength->create(toolBar, ui::WsNone, new ui::TableLayout(L"100,35", L"24", 2, 2));

	m_sliderStrength = new ui::Slider();
	m_sliderStrength->create(containerStrength);
	m_sliderStrength->setRange(1, 10);
	m_sliderStrength->setValue(5);
	m_sliderStrength->addChangeEventHandler(ui::createMethodHandler(this, &TerrainEditorPlugin::eventSliderStrengthChange));

	m_staticStrength = new ui::Static();
	m_staticStrength->create(containerStrength, L"50%");

	m_toolStrength = new ui::custom::ToolBarEmbed(containerStrength, 135);

	m_colorControl = new ui::custom::ColorControl();
	m_colorControl->create(toolBar, ui::WsBorder);
	m_colorControl->setColor(Color4ub(255, 255, 255, 255));
	m_colorControl->addButtonUpEventHandler(ui::createMethodHandler(this, &TerrainEditorPlugin::eventColorClick));

	m_toolColor = new ui::custom::ToolBarEmbed(m_colorControl, 32);

	m_toolMaterial = new ui::custom::ToolBarDropDown(ui::Command(L"Terrain.Editor.SelectMaterial"), 80, i18n::Text(L"TERRAIN_EDITOR_MATERIAL"));
	m_toolMaterial->add(L"Material 1");
	m_toolMaterial->add(L"Material 2");
	m_toolMaterial->add(L"Material 3");
	m_toolMaterial->add(L"Material 4");
	m_toolMaterial->select(0);
	
	m_toolToggleElevate->setToggled(true);
	m_toolToggleFallOffSmooth->setToggled(true);

	toolBar->addItem(new ui::custom::ToolBarSeparator());
	toolBar->addItem(m_toolToggleEditTerrain);
	toolBar->addItem(m_toolToggleMaterial);
	toolBar->addItem(m_toolToggleColor);
	toolBar->addItem(m_toolToggleElevate);
	toolBar->addItem(m_toolToggleFlatten);
	toolBar->addItem(m_toolToggleAverage);
	toolBar->addItem(m_toolToggleSmooth);
	toolBar->addItem(m_toolToggleNoise);
	toolBar->addItem(m_toolToggleCut);
	toolBar->addItem(new ui::custom::ToolBarSeparator());
	toolBar->addItem(m_toolToggleFallOffSmooth);
	toolBar->addItem(m_toolToggleFallOffSharp);
	toolBar->addItem(new ui::custom::ToolBarSeparator());
	toolBar->addItem(m_toolToggleSymmetryX);
	toolBar->addItem(m_toolToggleSymmetryZ);
	toolBar->addItem(new ui::custom::ToolBarSeparator());
	toolBar->addItem(m_toolStrength);
	toolBar->addItem(m_toolColor);
	toolBar->addItem(m_toolMaterial);

	updateModifierState();

	m_context->addModifierChangedEventHandler(ui::createMethodHandler(this, &TerrainEditorPlugin::eventModifierChanged));
	return true;
}

bool TerrainEditorPlugin::handleCommand(const ui::Command& command)
{
	{
		if (command == L"Terrain.Editor.EditTerrain")
		{
			m_context->setModifier(m_terrainEditModifier);
			return true;
		}
	}

	{
		ui::custom::ToolBarButton* toolSelected = 0;

		if (command == L"Terrain.Editor.MaterialBrush")
			toolSelected = m_toolToggleMaterial;
		else if (command == L"Terrain.Editor.ColorBrush")
			toolSelected = m_toolToggleColor;
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
		else if (command == L"Terrain.Editor.CutBrush")
			toolSelected = m_toolToggleCut;

		if (toolSelected)
		{
			m_toolToggleMaterial->setToggled(m_toolToggleMaterial == toolSelected);
			m_toolToggleColor->setToggled(m_toolToggleColor == toolSelected);
			m_toolToggleElevate->setToggled(m_toolToggleElevate == toolSelected);
			m_toolToggleFlatten->setToggled(m_toolToggleFlatten == toolSelected);
			m_toolToggleAverage->setToggled(m_toolToggleAverage == toolSelected);
			m_toolToggleSmooth->setToggled(m_toolToggleSmooth == toolSelected);
			m_toolToggleNoise->setToggled(m_toolToggleNoise == toolSelected);
			m_toolToggleCut->setToggled(m_toolToggleCut == toolSelected);
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

		if (toolSelected)
		{
			m_toolToggleFallOffSmooth->setToggled(m_toolToggleFallOffSmooth == toolSelected);
			m_toolToggleFallOffSharp->setToggled(m_toolToggleFallOffSharp == toolSelected);
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

	return false;
}

void TerrainEditorPlugin::updateModifierState()
{
	if (m_toolToggleMaterial->isToggled())
		m_terrainEditModifier->setBrush(L"Terrain.Editor.MaterialBrush");
	else if (m_toolToggleColor->isToggled())
		m_terrainEditModifier->setBrush(L"Terrain.Editor.ColorBrush");
	else if (m_toolToggleElevate->isToggled())
		m_terrainEditModifier->setBrush(L"Terrain.Editor.ElevateBrush");
	else if (m_toolToggleFlatten->isToggled())
		m_terrainEditModifier->setBrush(L"Terrain.Editor.FlattenBrush");
	else if (m_toolToggleAverage->isToggled())
		m_terrainEditModifier->setBrush(L"Terrain.Editor.AverageBrush");
	else if (m_toolToggleSmooth->isToggled())
		m_terrainEditModifier->setBrush(L"Terrain.Editor.SmoothBrush");
	else if (m_toolToggleNoise->isToggled())
		m_terrainEditModifier->setBrush(L"Terrain.Editor.NoiseBrush");
	else if (m_toolToggleCut->isToggled())
		m_terrainEditModifier->setBrush(L"Terrain.Editor.CutBrush");

	if (m_toolToggleFallOffSmooth->isToggled())
		m_terrainEditModifier->setFallOff(L"Terrain.Editor.SmoothFallOff");
	else if (m_toolToggleFallOffSharp->isToggled())
		m_terrainEditModifier->setFallOff(L"Terrain.Editor.SharpFallOff");

	uint32_t symmetry = 0;
	if (m_toolToggleSymmetryX->isToggled())
		symmetry |= 1;
	if (m_toolToggleSymmetryZ->isToggled())
		symmetry |= 2;
	m_terrainEditModifier->setSymmetry(symmetry);

	int32_t material = m_toolMaterial->getSelected();
	if (material >= 0)
		m_terrainEditModifier->setMaterial(material);

	m_terrainEditModifier->setStrength(m_sliderStrength->getValue() / 10.0f);
}

void TerrainEditorPlugin::eventSliderStrengthChange(ui::Event* event)
{
	m_terrainEditModifier->setStrength(m_sliderStrength->getValue() / 10.0f);
	m_staticStrength->setText(toString(int32_t(m_sliderStrength->getValue() * 10)) + L"%");
}

void TerrainEditorPlugin::eventColorClick(ui::Event* event)
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

void TerrainEditorPlugin::eventModifierChanged(ui::Event* event)
{
	m_toolToggleEditTerrain->setToggled(m_context->getModifier() == m_terrainEditModifier);
}

	}
}
