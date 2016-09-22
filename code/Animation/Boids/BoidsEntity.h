#ifndef traktor_animation_BoidsEntity_H
#define traktor_animation_BoidsEntity_H

#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "World/Entity.h"

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

class IWorldRenderPass;
class WorldContext;
class WorldRenderView;

	}

	namespace animation
	{

/*! \brief
 * \ingroup Animation
 */
class T_DLLCLASS BoidsEntity : public world::Entity
{
	T_RTTI_CLASS;

public:
	BoidsEntity(
		const RefArray< world::Entity >& boidEntities,
		const Transform& transform,
		const Vector4& spawnPositionDiagonal,
		const Vector4& spawnVelocityDiagonal,
		float followForce,
		float repelDistance,
		float repelForce,
		float matchVelocityStrength,
		float centerForce,
		float maxVelocity
	);

	virtual ~BoidsEntity();

	virtual void destroy() T_OVERRIDE T_FINAL;

	void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass
	);

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual bool getTransform(Transform& outTransform) const T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual void update(const world::UpdateParams& update) T_OVERRIDE T_FINAL;

private:
	struct Boid
	{
		Vector4 position;
		Vector4 velocity;
	};

	RefArray< world::Entity > m_boidEntities;
	AlignedVector< Boid > m_boids;
	Transform m_transform;
	float m_followForce;
	float m_repelDistance;
	float m_repelForce;
	float m_matchVelocityStrength;
	float m_centerForce;
	float m_maxVelocity;
};

	}
}

#endif	// traktor_animation_BoidsEntity_H
