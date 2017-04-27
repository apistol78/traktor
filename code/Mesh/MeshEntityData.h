/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_mesh_MeshEntityData_H
#define traktor_mesh_MeshEntityData_H

#include "Core/Math/Matrix44.h"
#include "Mesh/AbstractMeshEntityData.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class IMesh;

class T_DLLCLASS MeshEntityData : public AbstractMeshEntityData
{
	T_RTTI_CLASS;

public:
	MeshEntityData();

	void setMesh(const resource::Id< IMesh >& mesh);

	const resource::Id< IMesh >& getMesh() const;

	virtual Ref< MeshEntity > createEntity(resource::IResourceManager* resourceManager, const world::IEntityBuilder* builder) const T_OVERRIDE T_FINAL;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	resource::Id< IMesh > m_mesh;
	bool m_screenSpaceCulling;
};

	}
}

#endif	// traktor_mesh_MeshEntityData_H
