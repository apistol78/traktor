#ifndef traktor_mesh_MeshComponent_H
#define traktor_mesh_MeshComponent_H

#include "Core/Math/IntervalTransform.h"
#include "World/IEntityComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class Entity;
class IWorldRenderPass;
class WorldContext;
class WorldRenderView;

	}

	namespace mesh
	{

class IMeshParameterCallback;

/*! \brief
 * \ingroup Mesh
 */
class T_DLLCLASS MeshComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	MeshComponent(world::Entity* owner, bool screenSpaceCulling);

	virtual void destroy() T_OVERRIDE;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual void update(const world::UpdateParams& update) T_OVERRIDE T_FINAL;

	virtual void render(world::WorldContext& worldContext, world::WorldRenderView& worldRenderView, world::IWorldRenderPass& worldRenderPass) = 0;

	const IntervalTransform& getTransform() const { return m_transform; }

protected:
	world::Entity* m_owner;
	Ref< const IMeshParameterCallback > m_parameterCallback;
	bool m_screenSpaceCulling;
	IntervalTransform m_transform;
};

	}
}

#endif	// traktor_mesh_MeshComponent_H
