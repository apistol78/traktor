#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IDocument.h"
#include "Editor/IEditor.h"
#include "Heightfield/Editor/HeightfieldAsset.h"
#include "Heightfield/Editor/HeightfieldCompositor.h"
#include "Heightfield/Editor/HeightfieldEditorPlugin.h"
#include "Scene/Editor/SceneEditorContext.h"

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
	return true;
}

bool HeightfieldEditorPlugin::handleCommand(const ui::Command& command)
{
	// Editor is about to save the document; need to write
	// height-field into instance's data first.
	if (command == L"Editor.ShouldSave")
		writeInstanceData();
	return false;
}

HeightfieldCompositor* HeightfieldEditorPlugin::getCompositor(const Guid& assetGuid)
{
	std::map< Guid, Ref< HeightfieldCompositor > >::iterator i = m_compositors.find(assetGuid);
	if (i != m_compositors.end())
		return i->second;

	db::Database* sourceDatabase = m_context->getSourceDatabase();
	if (!sourceDatabase)
		return 0;

	std::wstring assetPath = m_context->getEditor()->getSettings()->getProperty< PropertyString >(L"Pipeline.AssetPath", L"");

	Ref< db::Instance > assetInstance = sourceDatabase->getInstance(assetGuid);
	if (!assetInstance)
		return 0;

	if (!assetInstance->checkout())
		return 0;

	Ref< HeightfieldAsset > asset = assetInstance->getObject< HeightfieldAsset >();
	if (!asset)
	{
		assetInstance->revert();
		return 0;
	}

	Ref< HeightfieldCompositor > compositor = HeightfieldCompositor::createFromAsset(asset, assetPath);
	if (!compositor)
	{
		assetInstance->revert();
		return 0;
	}

	if (!compositor->readInstanceData(assetInstance))
	{
		assetInstance->revert();
		return 0;
	}

	m_context->getDocument()->editInstance(assetInstance, asset);

	m_compositors.insert(std::make_pair(assetGuid, compositor));
	return compositor;
}

bool HeightfieldEditorPlugin::writeInstanceData()
{
	db::Database* sourceDatabase = m_context->getSourceDatabase();
	if (!sourceDatabase)
		return false;

	for (std::map< Guid, Ref< HeightfieldCompositor > >::const_iterator i = m_compositors.begin(); i != m_compositors.end(); ++i)
	{
		Ref< db::Instance > assetInstance = sourceDatabase->getInstance(i->first);
		if (!assetInstance)
			return false;

		if (!i->second->writeInstanceData(assetInstance))
			return false;
	}

	return true;
}

	}
}
