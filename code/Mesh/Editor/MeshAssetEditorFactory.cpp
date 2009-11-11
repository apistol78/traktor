#include "Mesh/Editor/MeshAssetEditorFactory.h"
#include "Mesh/Editor/MeshAssetEditor.h"
#include "Mesh/Editor/MeshAsset.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.mesh.MeshAssetEditorFactory", MeshAssetEditorFactory, editor::IObjectEditorFactory)

const TypeSet MeshAssetEditorFactory::getEditableTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< MeshAsset >());
	return typeSet;
}

Ref< editor::IObjectEditor > MeshAssetEditorFactory::createObjectEditor(editor::IEditor* editor) const
{
	return gc_new< MeshAssetEditor >(editor);
}

	}
}
