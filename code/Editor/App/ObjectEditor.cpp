/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Editor/App/ObjectEditor.h"
#include "Ui/Dialog.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.ObjectEditor", ObjectEditor, Object)

ObjectEditor::ObjectEditor(IEditor* editor, ui::Dialog* parent)
:	m_editor(editor)
,	m_parent(parent)
{
}

Ref< const PropertyGroup > ObjectEditor::getSettings() const
{
	return m_editor->getSettings();
}

Ref< PropertyGroup > ObjectEditor::checkoutGlobalSettings()
{
	return m_editor->checkoutGlobalSettings();
}

void ObjectEditor::commitGlobalSettings()
{
	m_editor->commitGlobalSettings();
}

void ObjectEditor::revertGlobalSettings()
{
	m_editor->revertGlobalSettings();
}

Ref< PropertyGroup > ObjectEditor::checkoutWorkspaceSettings()
{
	return m_editor->checkoutWorkspaceSettings();
}

void ObjectEditor::commitWorkspaceSettings()
{
	m_editor->commitWorkspaceSettings();
}

void ObjectEditor::revertWorkspaceSettings()
{
	m_editor->revertWorkspaceSettings();
}

Ref< ILogTarget > ObjectEditor::createLogTarget(const std::wstring& title)
{
	return m_editor->createLogTarget(title);
}

Ref< db::Database > ObjectEditor::getSourceDatabase() const
{
	return m_editor->getSourceDatabase();
}

Ref< db::Database > ObjectEditor::getOutputDatabase() const
{
	return m_editor->getOutputDatabase();
}

void ObjectEditor::updateDatabaseView()
{
	m_editor->updateDatabaseView();
}

bool ObjectEditor::highlightInstance(const db::Instance* instance)
{
	return m_editor->highlightInstance(instance);
}

const TypeInfo* ObjectEditor::browseType()
{
	m_parent->setEnable(false);
	const TypeInfo* browsedType = m_editor->browseType();
	m_parent->setEnable(true);
	return browsedType;
}

const TypeInfo* ObjectEditor::browseType(const TypeInfoSet& base)
{
	m_parent->setEnable(false);
	const TypeInfo* browsedType = m_editor->browseType(base);
	m_parent->setEnable(true);
	return browsedType;
}

Ref< db::Instance > ObjectEditor::browseInstance(const TypeInfo& filterType)
{
	m_parent->setEnable(false);
	Ref< db::Instance > browsedInstance = m_editor->browseInstance(filterType);
	m_parent->setEnable(true);
	return browsedInstance;
}

Ref< db::Instance > ObjectEditor::browseInstance(const IBrowseFilter* filter)
{
	m_parent->setEnable(false);
	Ref< db::Instance > browsedInstance = m_editor->browseInstance(filter);
	m_parent->setEnable(true);
	return browsedInstance;
}

bool ObjectEditor::openEditor(db::Instance* instance)
{
	return m_editor->openEditor(instance);
}

bool ObjectEditor::openDefaultEditor(db::Instance* instance)
{
	return m_editor->openDefaultEditor(instance);
}

bool ObjectEditor::openTool(const std::wstring& toolType, const std::wstring& param)
{
	return m_editor->openTool(toolType, param);
}

bool ObjectEditor::openBrowser(const net::Url& url)
{
	return m_editor->openBrowser(url);
}

Ref< IEditorPage > ObjectEditor::getActiveEditorPage()
{
	return m_editor->getActiveEditorPage();
}

void ObjectEditor::setActiveEditorPage(IEditorPage* editorPage)
{
	m_editor->setActiveEditorPage(editorPage);
}

void ObjectEditor::buildAssets(const std::vector< Guid >& assetGuids, bool rebuild)
{
	m_editor->buildAssets(assetGuids, rebuild);
}

void ObjectEditor::buildAsset(const Guid& assetGuid, bool rebuild)
{
	m_editor->buildAsset(assetGuid, rebuild);
}

void ObjectEditor::buildAssets(bool rebuild)
{
	m_editor->buildAssets(rebuild);
}

Ref< IPipelineDependencySet > ObjectEditor::buildAssetDependencies(const ISerializable* asset, uint32_t recursionDepth)
{
	return m_editor->buildAssetDependencies(asset, recursionDepth);
}

void ObjectEditor::setStoreObject(const std::wstring& name, Object* object)
{
	m_editor->setStoreObject(name, object);
}

Object* ObjectEditor::getStoreObject(const std::wstring& name) const
{
	return m_editor->getStoreObject(name);
}

	}
}