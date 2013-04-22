#include "Core/Misc/String.h"
#include "I18N/Text.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Terrain/Editor/TerrainEditorPlugin.h"
#include "Terrain/Editor/TerrainEditModifier.h"
#include "Ui/Bitmap.h"
#include "Ui/Container.h"
#include "Ui/MethodHandler.h"
#include "Ui/Slider.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
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
	int32_t image = toolBar->addImage(ui::Bitmap::load(c_ResourceTerrain, sizeof(c_ResourceTerrain), L"png"), 8);

	m_toolToggleEditTerrain = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_EDIT_TERRAIN"), ui::Command(L"Terrain.Editor.EditTerrain"), image + 6, ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleElevate = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_ELEVATE_BRUSH"), ui::Command(L"Terrain.Editor.ElevateBrush"), image + 0, ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleFlatten = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_FLATTEN_BRUSH"), ui::Command(L"Terrain.Editor.FlattenBrush"), image + 1, ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleAverage = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_AVERAGE_BRUSH"), ui::Command(L"Terrain.Editor.AverageBrush"), image + 3, ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleSmooth = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_SMOOTH_BRUSH"), ui::Command(L"Terrain.Editor.SmoothBrush"), image + 2, ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleCut = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_CUT_BRUSH"), ui::Command(L"Terrain.Editor.CutBrush"), image + 7, ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleFallOffSmooth = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_SMOOTH_FALLOFF"), ui::Command(L"Terrain.Editor.SmoothFallOff"), image + 4, ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolToggleFallOffSharp = new ui::custom::ToolBarButton(i18n::Text(L"TERRAIN_EDITOR_SHARP_FALLOFF"), ui::Command(L"Terrain.Editor.SharpFallOff"), image + 5, ui::custom::ToolBarButton::BsDefaultToggle);

	Ref< ui::Container > containerStrength = new ui::Container();
	containerStrength->create(toolBar, ui::WsBorder, new ui::TableLayout(L"100,40", L"24", 2, 2));

	m_sliderStrength = new ui::Slider();
	m_sliderStrength->create(containerStrength);
	m_sliderStrength->setRange(1, 10);
	m_sliderStrength->setValue(5);
	m_sliderStrength->addChangeEventHandler(ui::createMethodHandler(this, &TerrainEditorPlugin::eventSliderStrengthChange));

	m_staticStrength = new ui::Static();
	m_staticStrength->create(containerStrength, L"50%");

	m_toolStrength = new ui::custom::ToolBarEmbed(containerStrength, 140);
	
	m_toolToggleElevate->setToggled(true);
	m_toolToggleFallOffSmooth->setToggled(true);

	toolBar->addItem(new ui::custom::ToolBarSeparator());
	toolBar->addItem(m_toolToggleEditTerrain);
	toolBar->addItem(m_toolToggleElevate);
	toolBar->addItem(m_toolToggleFlatten);
	toolBar->addItem(m_toolToggleAverage);
	toolBar->addItem(m_toolToggleSmooth);
	toolBar->addItem(m_toolToggleCut);
	toolBar->addItem(new ui::custom::ToolBarSeparator());
	toolBar->addItem(m_toolToggleFallOffSmooth);
	toolBar->addItem(m_toolToggleFallOffSharp);
	toolBar->addItem(new ui::custom::ToolBarSeparator());
	toolBar->addItem(m_toolStrength);

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

		if (command == L"Terrain.Editor.ElevateBrush")
			toolSelected = m_toolToggleElevate;
		else if (command == L"Terrain.Editor.FlattenBrush")
			toolSelected = m_toolToggleFlatten;
		else if (command == L"Terrain.Editor.AverageBrush")
			toolSelected = m_toolToggleAverage;
		else if (command == L"Terrain.Editor.SmoothBrush")
			toolSelected = m_toolToggleSmooth;
		else if (command == L"Terrain.Editor.CutBrush")
			toolSelected = m_toolToggleCut;

		if (toolSelected)
		{
			m_toolToggleElevate->setToggled(m_toolToggleElevate == toolSelected);
			m_toolToggleFlatten->setToggled(m_toolToggleFlatten == toolSelected);
			m_toolToggleAverage->setToggled(m_toolToggleAverage == toolSelected);
			m_toolToggleSmooth->setToggled(m_toolToggleSmooth == toolSelected);
			m_toolToggleCut->setToggled(m_toolToggleCut == toolSelected);
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
		}
	}

	return false;
}

void TerrainEditorPlugin::eventSliderStrengthChange(ui::Event* event)
{
	TerrainEditModifier* modifier = dynamic_type_cast< TerrainEditModifier* >(m_context->getModifier());
	if (modifier)
		modifier->setStrength(m_sliderStrength->getValue() / 10.0f);
	m_staticStrength->setText(toString(int32_t(m_sliderStrength->getValue() * 10)) + L"%");
}

void TerrainEditorPlugin::eventModifierChanged(ui::Event* event)
{
	m_toolToggleEditTerrain->setToggled(m_context->getModifier() == m_terrainEditModifier);
}

	}
}
