#include "Core/Settings/PropertyString.h"
#include "Core/Settings/Settings.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Heightfield/Editor/HeightfieldCompositor.h"
#include "Heightfield/Editor/HeightfieldEditorPlugin.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Bitmap.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"

// Resources
#include "Resources/Heightfield.h"

namespace traktor
{
	namespace hf
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.hf.HeightfieldEditorPlugin", HeightfieldEditorPlugin, scene::ISceneEditorPlugin)

HeightfieldEditorPlugin::HeightfieldEditorPlugin(scene::SceneEditorContext* context)
:	m_context(context)
{
}

bool HeightfieldEditorPlugin::create(ui::Widget* parent, ui::custom::ToolBar* toolBar)
{
	uint32_t base = toolBar->addImage(ui::Bitmap::load(c_ResourceHeightfield, sizeof(c_ResourceHeightfield), L"png"), 6);
	toolBar->addItem(new ui::custom::ToolBarSeparator());
	toolBar->addItem(new ui::custom::ToolBarButton(L"Heightfield raise", ui::Command(L"Heightfield.Raise"), base));
	toolBar->addItem(new ui::custom::ToolBarButton(L"Heightfield lower", ui::Command(L"Heightfield.Lower"), base + 1));
	toolBar->addItem(new ui::custom::ToolBarButton(L"Heightfield save", ui::Command(L"Heightfield.Save"), base + 2));
	return true;
}

bool HeightfieldEditorPlugin::handleCommand(const ui::Command& command)
{
	if (command == L"Heightfield.Save")
	{
		saveCompositors();
		return true;
	}
	return false;
}

HeightfieldCompositor* HeightfieldEditorPlugin::getCompositor(const Guid& assetGuid)
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

bool HeightfieldEditorPlugin::saveCompositors()
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
