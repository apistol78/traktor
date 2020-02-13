#pragma once

#include "Core/Ref.h"
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
class WorldBuildContext;
class WorldRenderView;

	}

	namespace mesh
	{

class IMeshParameterCallback;

/*! Abstract mesh component.
 * \ingroup Mesh
 */
class T_DLLCLASS MeshComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	MeshComponent(bool screenSpaceCulling);

	virtual void destroy() override;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override;

	virtual void update(const world::UpdateParams& update) override;

	virtual void build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass) = 0;

	void setParameterCallback(const IMeshParameterCallback* parameterCallback) { m_parameterCallback = parameterCallback; }

	const IMeshParameterCallback* getParameterCallback() const { return m_parameterCallback; }

	const IntervalTransform& getTransform() const { return m_transform; }

	IntervalTransform& getTransform() { return m_transform; }

protected:
	world::Entity* m_owner;
	Ref< const IMeshParameterCallback > m_parameterCallback;
	bool m_screenSpaceCulling;
	IntervalTransform m_transform;
};

	}
}

