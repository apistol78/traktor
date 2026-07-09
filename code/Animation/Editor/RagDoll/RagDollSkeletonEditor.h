/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Editor/IObjectEditor.h"
#include "Ui/PropertyList/AutoPropertyList.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::editor
{

class IEditor;

}

namespace traktor
{

class ISerializable;

}

namespace traktor::ui
{

class MouseButtonDownEvent;
class SelectionChangeEvent;
class ToolBar;
class ToolBarButtonClickEvent;
class TreeView;
class TreeViewItem;
class PropertyCommandEvent;
class PropertyContentChangeEvent;

}

namespace traktor::animation
{

class RagDollBone;
class RagDollSkeletonAsset;
class RagDollPreviewControl;

/*! Rag doll skeleton asset editor.
 * \ingroup Animation
 *
 * Presents a property view over the RagDollSkeletonAsset alongside an
 * interactive 3D preview of the setup, optionally overlaid with an
 * animation skeleton for reference.
 */
class T_DLLCLASS RagDollSkeletonEditor
:	public editor::IObjectEditor
,	public ui::PropertyList::IPropertyGuidResolver
{
	T_RTTI_CLASS;

public:
	explicit RagDollSkeletonEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object) override final;

	virtual void destroy() override final;

	virtual void apply() override final;

	virtual bool handleCommand(const ui::Command& command) override final;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) override final;

	virtual ui::Size getPreferredSize() const override final;

private:
	editor::IEditor* m_editor;
	std::wstring m_assetPath;
	std::wstring m_modelCachePath;
	Ref< db::Instance > m_instance;
	Ref< RagDollSkeletonAsset > m_asset;
	Ref< ui::TreeView > m_treeView;
	Ref< ui::AutoPropertyList > m_propertyList;
	Ref< ui::ToolBar > m_toolBar;
	Ref< RagDollPreviewControl > m_previewControl;

	virtual bool resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const override final;

	void updateTreeView();

	void buildBoneItems(ui::TreeViewItem* parentItem, RagDollBone* bone);

	void selectTreeItem(const ISerializable* object);

	void browseSkeleton();

	void eventPropertyCommand(ui::PropertyCommandEvent* event);

	void eventPropertyChange(ui::PropertyContentChangeEvent* event);

	void eventToolBarClick(ui::ToolBarButtonClickEvent* event);

	void eventTreeSelect(ui::SelectionChangeEvent* event);

	void eventTreeButtonDown(ui::MouseButtonDownEvent* event);
};

}
