/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_mesh_IndoorMeshEntity_H
#define traktor_mesh_IndoorMeshEntity_H

#include "Mesh/MeshEntity.h"
#include "Resource/Proxy.h"

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

class IndoorMesh;

class T_DLLCLASS IndoorMeshEntity : public MeshEntity
{
	T_RTTI_CLASS;

public:
	IndoorMeshEntity(const Transform& transform, bool screenSpaceCulling, const resource::Proxy< IndoorMesh >& mesh);

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual bool supportTechnique(render::handle_t technique) const T_OVERRIDE T_FINAL;

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		float distance
	) T_OVERRIDE T_FINAL;

private:
	resource::Proxy< IndoorMesh > m_mesh;
};

	}
}

#endif	// traktor_mesh_IndoorMeshEntity_H
