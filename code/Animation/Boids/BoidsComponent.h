#pragma once

#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "World/IEntityComponent.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
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

	namespace animation
	{

/*!
 * \ingroup Animation
 */
class T_DLLCLASS BoidsComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	BoidsComponent(
		const RefArray< world::Entity >& boidEntities,
		const Vector4& spawnPositionDiagonal,
		const Vector4& spawnVelocityDiagonal,
		const Vector4& constrain,
		float followForce,
		float repelDistance,
		float repelForce,
		float matchVelocityStrength,
		float centerForce,
		float maxVelocity
	);

	virtual ~BoidsComponent();

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	void build(
		const world::WorldBuildContext& context,
		const world::WorldRenderView& worldRenderView,
		const world::IWorldRenderPass& worldRenderPass
	);

private:
	struct Boid
	{
		Vector4 position;
		Vector4 velocity;
	};

	RefArray< world::Entity > m_boidEntities;
	AlignedVector< Boid > m_boids;
	Transform m_transform;
	Vector4 m_constrain;
	float m_followForce;
	float m_repelDistance;
	float m_repelForce;
	float m_matchVelocityStrength;
	float m_centerForce;
	float m_maxVelocity;
};

	}
}

