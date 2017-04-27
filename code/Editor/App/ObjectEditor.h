/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_ObjectEditor_H
#define traktor_editor_ObjectEditor_H

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

	virtual Ref< const PropertyGroup > getOriginalSettings() const T_OVERRIDE T_FINAL;

	virtual Ref< const PropertyGroup > getSettings() const T_OVERRIDE T_FINAL;

	virtual Ref< const PropertyGroup > getGlobalSettings() const T_OVERRIDE T_FINAL;

	virtual Ref< const PropertyGroup > getWorkspaceSettings() const T_OVERRIDE T_FINAL;

	virtual Ref< PropertyGroup > checkoutGlobalSettings() T_OVERRIDE T_FINAL;

	virtual void commitGlobalSettings() T_OVERRIDE T_FINAL;

	virtual void revertGlobalSettings() T_OVERRIDE T_FINAL;

	virtual Ref< PropertyGroup > checkoutWorkspaceSettings() T_OVERRIDE T_FINAL;

	virtual void commitWorkspaceSettings() T_OVERRIDE T_FINAL;

	virtual void revertWorkspaceSettings() T_OVERRIDE T_FINAL;

	virtual Ref< ILogTarget > createLogTarget(const std::wstring& title) T_OVERRIDE T_FINAL;

	virtual Ref< db::Database > getSourceDatabase() const T_OVERRIDE T_FINAL;

	virtual Ref< db::Database > getOutputDatabase() const T_OVERRIDE T_FINAL;

	virtual void updateDatabaseView() T_OVERRIDE T_FINAL;

	virtual bool highlightInstance(const db::Instance* instance) T_OVERRIDE T_FINAL;

	virtual const TypeInfo* browseType() T_OVERRIDE T_FINAL;

	virtual const TypeInfo* browseType(const TypeInfoSet& base) T_OVERRIDE T_FINAL;

	virtual Ref< db::Instance > browseInstance(const TypeInfo& filterType) T_OVERRIDE T_FINAL;

	virtual Ref< db::Instance > browseInstance(const IBrowseFilter* filter) T_OVERRIDE T_FINAL;

	virtual bool openEditor(db::Instance* instance) T_OVERRIDE T_FINAL;

	virtual bool openDefaultEditor(db::Instance* instance) T_OVERRIDE T_FINAL;

	virtual bool openTool(const std::wstring& toolType, const std::wstring& param) T_OVERRIDE T_FINAL;

	virtual bool openBrowser(const net::Url& url) T_OVERRIDE T_FINAL;

	virtual Ref< IEditorPage > getActiveEditorPage() T_OVERRIDE T_FINAL;

	virtual void setActiveEditorPage(IEditorPage* editorPage) T_OVERRIDE T_FINAL;

	virtual void buildAssets(const std::vector< Guid >& assetGuids, bool rebuild) T_OVERRIDE T_FINAL;

	virtual void buildAsset(const Guid& assetGuid, bool rebuild) T_OVERRIDE T_FINAL;

	virtual void buildAssets(bool rebuild) T_OVERRIDE T_FINAL;

	virtual Ref< IPipelineDependencySet > buildAssetDependencies(const ISerializable* asset, uint32_t recursionDepth) T_OVERRIDE T_FINAL;

	virtual void setStoreObject(const std::wstring& name, Object* object) T_OVERRIDE T_FINAL;

	virtual Object* getStoreObject(const std::wstring& name) const T_OVERRIDE T_FINAL;

private:
	IEditor* m_editor;
	ui::Dialog* m_parent;
};

	}
}

#endif	// traktor_editor_ObjectEditor_H
