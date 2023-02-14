/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Editor/IObjectEditor.h"
#include "Ui/Events/AllEvents.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::editor
{

class IEditor;

}

namespace traktor::ui
{

class Container;
class CheckBox;
class DropDown;
class Edit;
class GridView;
class Slider;
class Static;
class ToolBarButtonClickEvent;

}

namespace traktor::model
{

class Model;

}

namespace traktor::physics
{

class MeshAsset;

class T_DLLCLASS MeshAssetEditor : public editor::IObjectEditor
{
	T_RTTI_CLASS;

public:
	MeshAssetEditor(editor::IEditor* editor);

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
	Ref< MeshAsset > m_asset;
	Ref< model::Model > m_model;
	Ref< ui::Edit > m_editFileName;
	//Ref< ui::Edit > m_editImportFilter;
	Ref< ui::Container > m_containerMaterials;
	//Ref< ui::DropDown > m_dropMeshType;
	Ref< ui::CheckBox > m_checkHull;
	//Ref< ui::CheckBox > m_checkCenter;
	//Ref< ui::Static > m_staticLodSteps;
	//Ref< ui::Slider > m_sliderLodSteps;
	Ref< ui::Edit > m_editMargin;
	//Ref< ui::Edit > m_editLodCullDistance;
	//Ref< ui::Edit > m_editScaleFactor;
	Ref< ui::GridView > m_materialList;
	//Ref< ui::GridView > m_materialTextureList;

	void updateModel();

	void updateFile();

	void updateMaterialList();

	void browseMaterial();

	void removeMaterial();

	void eventBrowseClick(ui::ButtonClickEvent* event);

	void eventPreviewModelClick(ui::ButtonClickEvent* event);

	void eventEditModelClick(ui::ButtonClickEvent* event);

	void eventMaterialToolClick(ui::ToolBarButtonClickEvent* event);

	void eventMaterialListDoubleClick(ui::MouseDoubleClickEvent* event);
};

}
