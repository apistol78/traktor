#pragma once

#include "Core/Object.h"
#include "Editor/IEditor.h"

namespace traktor
{
	namespace ui
	{

class Dialog;

	}

	namespace editor
	{

class ObjectEditor
:	public IEditor
,	public Object
{
	T_RTTI_CLASS;

public:
	ObjectEditor(IEditor* editor, ui::Dialog* parent);

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

	virtual bool openTool(const std::wstring& toolType, const PropertyGroup* param) override final;

	virtual IEditorPage* getActiveEditorPage() override final;

	virtual void setActiveEditorPage(IEditorPage* editorPage) override final;

	virtual void buildAssets(const std::vector< Guid >& assetGuids, bool rebuild) override final;

	virtual void buildAsset(const Guid& assetGuid, bool rebuild) override final;

	virtual void buildAssets(bool rebuild) override final;

	virtual void buildCancel() override final;

	virtual void buildWaitUntilFinished() override final;

	virtual Ref< IPipelineDepends> createPipelineDepends(PipelineDependencySet* dependencySet, uint32_t recursionDepth) override final;

	virtual void setStoreObject(const std::wstring& name, Object* object) override final;

	virtual Object* getStoreObject(const std::wstring& name) const override final;

private:
	IEditor* m_editor;
	ui::Dialog* m_parent;
};

	}
}
