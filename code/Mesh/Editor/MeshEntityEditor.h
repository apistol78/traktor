/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_mesh_MeshEntityEditor_H
#define traktor_mesh_MeshEntityEditor_H

#include "Scene/Editor/DefaultEntityEditor.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class T_DLLCLASS MeshEntityEditor : public scene::DefaultEntityEditor
{
	T_RTTI_CLASS;

public:
	MeshEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter);

	virtual bool isPickable() const T_OVERRIDE T_FINAL;

	virtual bool queryRay(
		const Vector4& worldRayOrigin,
		const Vector4& worldRayDirection,
		Scalar& outDistance
	) const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_mesh_MeshEntityEditor_H
