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

	virtual Ref< const PropertyGroup > getSettings() const;

	virtual Ref< const PropertyGroup > getGlobalSettings() const;

	virtual Ref< const PropertyGroup > getWorkspaceSettings() const;

	virtual Ref< PropertyGroup > checkoutGlobalSettings();

	virtual void commitGlobalSettings();

	virtual void revertGlobalSettings();

	virtual Ref< PropertyGroup > checkoutWorkspaceSettings();

	virtual void commitWorkspaceSettings();

	virtual void revertWorkspaceSettings();

	virtual Ref< ILogTarget > createLogTarget(const std::wstring& title);

	virtual Ref< db::Database > getSourceDatabase() const;

	virtual Ref< db::Database > getOutputDatabase() const;

	virtual void updateDatabaseView();

	virtual bool highlightInstance(const db::Instance* instance);

	virtual const TypeInfo* browseType(const TypeInfo* base);

	virtual Ref< db::Instance > browseInstance(const TypeInfo& filterType);

	virtual Ref< db::Instance > browseInstance(const IBrowseFilter* filter);

	virtual bool openEditor(db::Instance* instance);

	virtual bool openDefaultEditor(db::Instance* instance);

	virtual Ref< IEditorPage > getActiveEditorPage();

	virtual void setActiveEditorPage(IEditorPage* editorPage);

	virtual void buildAssets(const std::vector< Guid >& assetGuids, bool rebuild);

	virtual void buildAsset(const Guid& assetGuid, bool rebuild);

	virtual void buildAssets(bool rebuild);

	virtual bool buildAssetDependencies(const ISerializable* asset, uint32_t recursionDepth, RefArray< PipelineDependency >& outDependencies);

	virtual void setStoreObject(const std::wstring& name, Object* object);

	virtual Object* getStoreObject(const std::wstring& name) const;

private:
	IEditor* m_editor;
	ui::Dialog* m_parent;
};

	}
}

#endif	// traktor_editor_ObjectEditor_H
