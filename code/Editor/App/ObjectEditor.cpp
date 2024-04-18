/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Editor/App/ObjectEditor.h"
#include "Ui/Dialog.h"

namespace traktor::editor
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

Ref< ISerializable > ObjectEditor::cloneAsset(const ISerializable* asset) const
{
	return m_editor->cloneAsset(asset);
}

const TypeInfo* ObjectEditor::browseType()
{
	m_parent->setEnable(false);
	const TypeInfo* browsedType = m_editor->browseType();
	m_parent->setEnable(true);
	return browsedType;
}

const TypeInfo* ObjectEditor::browseType(const TypeInfoSet& base, bool onlyEditable, bool onlyInstantiable)
{
	m_parent->setEnable(false);
	const TypeInfo* browsedType = m_editor->browseType(base, onlyEditable, onlyInstantiable);
	m_parent->setEnable(true);
	return browsedType;
}

Ref< db::Group > ObjectEditor::browseGroup()
{
	m_parent->setEnable(false);
	Ref< db::Group > browsedGroup = m_editor->browseGroup();
	m_parent->setEnable(true);
	return browsedGroup;
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

bool ObjectEditor::openInNewEditor(db::Instance* instance)
{
	return m_editor->openInNewEditor(instance);
}

bool ObjectEditor::openTool(const std::wstring& toolType, const PropertyGroup* param)
{
	return m_editor->openTool(toolType, param);
}

IEditorPage* ObjectEditor::getActiveEditorPage()
{
	return m_editor->getActiveEditorPage();
}

void ObjectEditor::setActiveEditorPage(IEditorPage* editorPage)
{
	m_editor->setActiveEditorPage(editorPage);
}

void ObjectEditor::buildAssets(const AlignedVector< Guid >& assetGuids, bool rebuild)
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

void ObjectEditor::buildCancel()
{
	m_editor->buildCancel();
}

void ObjectEditor::buildWaitUntilFinished()
{
	m_editor->buildWaitUntilFinished();
}

bool ObjectEditor::isBuilding() const
{
	return m_editor->isBuilding();
}

Ref< IPipelineDepends> ObjectEditor::createPipelineDepends(PipelineDependencySet* dependencySet, uint32_t recursionDepth)
{
	return m_editor->createPipelineDepends(dependencySet, recursionDepth);
}

ObjectStore* ObjectEditor::getObjectStore()
{
	return m_editor->getObjectStore();
}

}
