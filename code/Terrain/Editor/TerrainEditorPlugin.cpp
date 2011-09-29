#include "Core/Settings/PropertyString.h"
#include "Core/Settings/Settings.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Terrain/Editor/HeightfieldCompositor.h"
#include "Terrain/Editor/TerrainEditorPlugin.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"

namespace traktor
{
	namespace terrain
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.TerrainEditorPlugin", TerrainEditorPlugin, scene::ISceneEditorPlugin)

TerrainEditorPlugin::TerrainEditorPlugin(scene::SceneEditorContext* context)
:	m_context(context)
{
}

bool TerrainEditorPlugin::create(ui::Widget* parent, ui::custom::ToolBar* toolBar)
{
	toolBar->addItem(new ui::custom::ToolBarSeparator());
	toolBar->addItem(new ui::custom::ToolBarButton(L"Terrain raise", ui::Command(L"Terrain.Raise"), 0, ui::custom::ToolBarButton::BsText));
	toolBar->addItem(new ui::custom::ToolBarButton(L"Terrain lower", ui::Command(L"Terrain.Lower"), 0, ui::custom::ToolBarButton::BsText));
	toolBar->addItem(new ui::custom::ToolBarButton(L"Terrain save", ui::Command(L"Terrain.Save"), 0, ui::custom::ToolBarButton::BsText));
	return true;
}

bool TerrainEditorPlugin::handleCommand(const ui::Command& command)
{
	if (command == L"Terrain.Save")
	{
		saveCompositors();
		return true;
	}
	return false;
}

HeightfieldCompositor* TerrainEditorPlugin::getCompositor(const Guid& assetGuid)
{
	std::map< Guid, Ref< HeightfieldCompositor > >::iterator i = m_compositors.find(assetGuid);
	if (i != m_compositors.end())
		return i->second;

	std::wstring assetPath = m_context->getEditor()->getSettings()->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");

	db::Database* sourceDatabase = m_context->getSourceDatabase();
	if (!sourceDatabase)
		return 0;

	Ref< db::Instance > assetInstance = sourceDatabase->getInstance(assetGuid);
	if (!assetInstance)
		return 0;

	Ref< HeightfieldCompositor > compositor = HeightfieldCompositor::createFromInstance(assetInstance, assetPath);
	if (!compositor)
		return 0;

	m_compositors.insert(std::make_pair(assetGuid, compositor));
	return compositor;
}

bool TerrainEditorPlugin::saveCompositors()
{
	db::Database* sourceDatabase = m_context->getSourceDatabase();
	if (!sourceDatabase)
		return false;

	std::vector< Guid > savedAssets;

	for (std::map< Guid, Ref< HeightfieldCompositor > >::const_iterator i = m_compositors.begin(); i != m_compositors.end(); ++i)
	{
		Ref< db::Instance > assetInstance = sourceDatabase->getInstance(i->first);
		if (!assetInstance)
			return false;

		if (!i->second->saveInstanceLayers(assetInstance))
			return false;

		savedAssets.push_back(i->first);
	}

	m_context->getEditor()->buildAssets(savedAssets, true);

	return true;
}

	}
}
