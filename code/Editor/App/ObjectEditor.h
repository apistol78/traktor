/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Editor/IEditor.h"

namespace traktor::ui
{

class Dialog;

}

namespace traktor::editor
{

class ObjectEditor
:	public IEditor
,	public Object
{
	T_RTTI_CLASS;

public:
	explicit ObjectEditor(IEditor* editor, ui::Dialog* parent);

	virtual Ref< const PropertyGroup > getSettings() const override final;

	virtual Ref< PropertyGroup > checkoutGlobalSettings() override final;

	virtual void commitGlobalSettings() override final;

	virtual void revertGlobalSettings() override final;

	virtual Ref< PropertyGroup > checkoutWorkspaceSettings() override final;

	virtual void commitWorkspaceSettings() override final;

	virtual void revertWorkspaceSettings() override final;

	virtual Ref< ILogTarget > createLogTarget(const std::wstring& title) override final;

	virtual Ref< db::Database > getSourceDatabase() const override final;

	virtual Ref< db::Database > getOutputDatabase() const override final;

	virtual void updateDatabaseView() override final;

	virtual bool highlightInstance(const db::Instance* instance) override final;

	virtual Ref< ISerializable > cloneAsset(const ISerializable* asset) const override final;

	virtual const TypeInfo* browseType() override final;

	virtual const TypeInfo* browseType(const TypeInfoSet& base, bool onlyEditable, bool onlyInstantiable) override final;

	virtual Ref< db::Group > browseGroup() override final;

	virtual Ref< db::Instance > browseInstance(const TypeInfo& filterType) override final;

	virtual Ref< db::Instance > browseInstance(const IBrowseFilter* filter) override final;

	virtual bool openEditor(db::Instance* instance) override final;

	virtual bool openDefaultEditor(db::Instance* instance) override final;

	virtual bool openInNewEditor(db::Instance* instance) override final;

	virtual bool openTool(const std::wstring& toolType, const PropertyGroup* param) override final;

	virtual IEditorPage* getActiveEditorPage() override final;

	virtual void setActiveEditorPage(IEditorPage* editorPage) override final;

	virtual void buildAssets(const AlignedVector< Guid >& assetGuids, bool rebuild) override final;

	virtual void buildAsset(const Guid& assetGuid, bool rebuild) override final;

	virtual void buildAssets(bool rebuild) override final;

	virtual void buildCancel() override final;

	virtual void buildWaitUntilFinished() override final;
	
	virtual bool isBuilding() const override final;

	virtual Ref< IPipelineDepends> createPipelineDepends(PipelineDependencySet* dependencySet, uint32_t recursionDepth) override final;

	virtual ObjectStore* getObjectStore() override final;

private:
	IEditor* m_editor;
	ui::Dialog* m_parent;
};

}
