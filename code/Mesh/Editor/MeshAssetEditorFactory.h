#ifndef traktor_mesh_MeshAssetEditorFactory_H
#define traktor_mesh_MeshAssetEditorFactory_H

#include "Editor/IObjectEditorFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class T_DLLCLASS MeshAssetEditorFactory : public editor::IObjectEditorFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEditableTypes() const;

	virtual Ref< editor::IObjectEditor > createObjectEditor(editor::IEditor* editor) const;
};

	}
}

#endif	// traktor_mesh_MeshAssetEditorFactory_H
