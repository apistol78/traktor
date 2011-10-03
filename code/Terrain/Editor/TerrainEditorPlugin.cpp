#include "Terrain/Editor/TerrainEditorPlugin.h"
#include "Ui/Bitmap.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"

// Resources
#include "Resources/Terrain.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainEditorPlugin", TerrainEditorPlugin, scene::ISceneEditorPlugin)

TerrainEditorPlugin::TerrainEditorPlugin()
:	m_selectedTool(0)
{
}

bool TerrainEditorPlugin::create(ui::Widget* parent, ui::custom::ToolBar* toolBar)
{
	uint32_t base = toolBar->addImage(ui::Bitmap::load(c_ResourceTerrain, sizeof(c_ResourceTerrain), L"png"), 6);
	toolBar->addItem(new ui::custom::ToolBarSeparator());

	m_toolRaiseTool = new ui::custom::ToolBarButton(L"Raise/lower tool", ui::Command(L"Terrain.RaiseLowerTool"), base, ui::custom::ToolBarButton::BsDefaultToggled);
	m_toolFlattenTool = new ui::custom::ToolBarButton(L"Flatten tool", ui::Command(L"Terrain.FlattenTool"), base + 1, ui::custom::ToolBarButton::BsDefaultToggle);
	m_toolSmoothTool = new ui::custom::ToolBarButton(L"Smooth tool", ui::Command(L"Terrain.SmoothTool"), base + 2, ui::custom::ToolBarButton::BsDefaultToggle);

	toolBar->addItem(m_toolRaiseTool);
	toolBar->addItem(m_toolFlattenTool);
	toolBar->addItem(m_toolSmoothTool);

	return true;
}

bool TerrainEditorPlugin::handleCommand(const ui::Command& command)
{
	if (command == L"Terrain.RaiseLowerTool")
	{
		m_toolRaiseTool->setToggled(true);
		m_toolFlattenTool->setToggled(false);
		m_toolSmoothTool->setToggled(false);
		m_selectedTool = 0;
	}
	else if (command == L"Terrain.FlattenTool")
	{
		m_toolRaiseTool->setToggled(false);
		m_toolFlattenTool->setToggled(true);
		m_toolSmoothTool->setToggled(false);
		m_selectedTool = 1;
	}
	else if (command == L"Terrain.SmoothTool")
	{
		m_toolRaiseTool->setToggled(false);
		m_toolFlattenTool->setToggled(false);
		m_toolSmoothTool->setToggled(true);
		m_selectedTool = 2;
	}
	return false;
}

	}
}
