/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_mesh_SkinnedMeshComponent_H
#define traktor_mesh_SkinnedMeshComponent_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Matrix44.h"
#include "Mesh/MeshComponent.h"
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

class SkinnedMesh;

/*! \brief
 * \ingroup Mesh
 */
class T_DLLCLASS SkinnedMeshComponent : public MeshComponent
{
	T_RTTI_CLASS;

public:
	SkinnedMeshComponent(const resource::Proxy< SkinnedMesh >& mesh, bool screenSpaceCulling);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual void render(world::WorldContext& worldContext, world::WorldRenderView& worldRenderView, world::IWorldRenderPass& worldRenderPass) T_OVERRIDE T_FINAL;

	void setJointTransforms(const AlignedVector< Matrix44 >& jointTransforms);

private:
	resource::Proxy< SkinnedMesh > m_mesh;
	AlignedVector< Vector4 > m_jointTransforms[2];
	int32_t m_count;
};

	}
}

#endif	// traktor_mesh_SkinnedMeshComponent_H
