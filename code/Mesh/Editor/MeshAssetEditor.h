/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_mesh_MeshAssetEditor_H
#define traktor_mesh_MeshAssetEditor_H

#include "Editor/IObjectEditor.h"
#include "Ui/Events/AllEvents.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class Container;
class CheckBox;
class DropDown;
class Edit;
class Slider;
class Static;

		namespace custom
		{

class GridView;
class ToolBarButtonClickEvent;

		}
	}

	namespace model
	{

class Model;

	}

	namespace mesh
	{

class MeshAsset;

class T_DLLCLASS MeshAssetEditor : public editor::IObjectEditor
{
	T_RTTI_CLASS;

public:
	MeshAssetEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void apply() T_OVERRIDE T_FINAL;

	virtual bool handleCommand(const ui::Command& command) T_OVERRIDE T_FINAL;

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId) T_OVERRIDE T_FINAL;

	virtual ui::Size getPreferredSize() const T_OVERRIDE T_FINAL;

private:
	editor::IEditor* m_editor;
	Ref< db::Instance > m_instance;
	Ref< MeshAsset > m_asset;
	Ref< model::Model > m_model;
	Ref< ui::Edit > m_editFileName;
	Ref< ui::Container > m_containerMaterials;
	Ref< ui::DropDown > m_dropMeshType;
	Ref< ui::CheckBox > m_checkBakeOcclusion;
	Ref< ui::CheckBox > m_checkCullDistantFaces;
	Ref< ui::Static > m_staticLodSteps;
	Ref< ui::Slider > m_sliderLodSteps;
	Ref< ui::Edit > m_editLodMaxDistance;
	Ref< ui::Edit > m_editLodCullDistance;
	Ref< ui::Edit > m_editScaleFactor;
	Ref< ui::custom::GridView > m_materialShaderList;
	Ref< ui::custom::GridView > m_materialTextureList;
	std::wstring m_assetPath;

	void updateModel();

	void updateFile();

	void updateMaterialList();

	void browseMaterialTemplate();
	
	void removeMaterialTemplate();

	void createMaterialShader();

	void browseMaterialShader();
	
	void removeMaterialShader();

	void createMaterialTexture();

	void browseMaterialTexture();

	void removeMaterialTexture();

	void eventMeshTypeChange(ui::SelectionChangeEvent* event);

	void eventLodStepsChange(ui::ContentChangeEvent* event);

	void eventBrowseClick(ui::ButtonClickEvent* event);

	void eventPreviewModelClick(ui::ButtonClickEvent* event);

	void eventEditModelClick(ui::ButtonClickEvent* event);

	void eventMaterialShaderToolClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventMaterialShaderListDoubleClick(ui::MouseDoubleClickEvent* event);

	void eventMaterialTextureToolClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventMaterialTextureListDoubleClick(ui::MouseDoubleClickEvent* event);
};

	}
}

#endif	// traktor_mesh_MeshAssetEditor_H
