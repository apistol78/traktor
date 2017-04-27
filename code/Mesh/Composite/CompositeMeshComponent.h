/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_mesh_CompositeMeshComponent_H
#define traktor_mesh_CompositeMeshComponent_H

#include "Core/RefArray.h"
#include "Mesh/MeshComponent.h"

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

/*! \brief
 * \ingroup Mesh
 */
class T_DLLCLASS CompositeMeshComponent : public MeshComponent
{
	T_RTTI_CLASS;

public:
	CompositeMeshComponent();

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual void update(const world::UpdateParams& update) T_OVERRIDE T_FINAL;

	virtual void render(world::WorldContext& worldContext, world::WorldRenderView& worldRenderView, world::IWorldRenderPass& worldRenderPass) T_OVERRIDE T_FINAL;

	void removeAll();

	void remove(MeshComponent* meshComponent);

	void add(MeshComponent* meshComponent);

private:
	RefArray< MeshComponent > m_meshComponents;
};

	}
}

#endif	// traktor_mesh_CompositeMeshComponent_H
