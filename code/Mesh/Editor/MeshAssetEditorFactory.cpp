#include "Mesh/Editor/MeshAssetEditorFactory.h"
#include "Mesh/Editor/MeshAssetEditor.h"
#include "Mesh/Editor/MeshAsset.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.MeshAssetEditorFactory", 0, MeshAssetEditorFactory, editor::IObjectEditorFactory)

const TypeInfoSet MeshAssetEditorFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< MeshAsset >());
	return typeSet;
}

Ref< editor::IObjectEditor > MeshAssetEditorFactory::createObjectEditor(editor::IEditor* editor) const
{
	return new MeshAssetEditor(editor);
}

	}
}
