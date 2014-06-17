#ifndef traktor_mesh_MeshAssetEditor_H
#define traktor_mesh_MeshAssetEditor_H

#include "Editor/IObjectEditor.h"

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
class Event;
class ListView;

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

	virtual bool create(ui::Widget* parent, db::Instance* instance, ISerializable* object);

	virtual void destroy();

	virtual void apply();

	virtual bool handleCommand(const ui::Command& command);

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId);

	virtual ui::Size getPreferredSize() const;

private:
	editor::IEditor* m_editor;
	Ref< db::Instance > m_instance;
	Ref< MeshAsset > m_asset;
	Ref< model::Model > m_model;
	Ref< ui::Edit > m_editFileName;
	Ref< ui::Container > m_containerMaterials;
	Ref< ui::DropDown > m_dropMeshType;
	Ref< ui::CheckBox > m_checkBakeOcclusion;
	Ref< ui::CheckBox > m_checkGenerateOccluder;
	Ref< ui::ListView > m_materialShaderList;
	Ref< ui::ListView > m_materialTextureList;
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

	void eventBrowseClick(ui::Event* event);

	void eventMaterialShaderToolClick(ui::Event* event);

	void eventMaterialShaderListDoubleClick(ui::Event* event);

	void eventMaterialTextureToolClick(ui::Event* event);

	void eventMaterialTextureListDoubleClick(ui::Event* event);
};

	}
}

#endif	// traktor_mesh_MeshAssetEditor_H
