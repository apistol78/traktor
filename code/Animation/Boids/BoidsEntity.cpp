#include "Animation/Boids/BoidsEntity.h"
#include "Core/Math/RandomGeometry.h"
#include "World/WorldContext.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.BoidsEntity", BoidsEntity, world::Entity)

BoidsEntity::BoidsEntity(
	const RefArray< world::Entity >& boidEntities,
	const Transform& transform,
	const Vector4& spawnPositionDiagonal,
	const Vector4& spawnVelocityDiagonal,
	const Vector4& constrain,
	float followForce,
	float repelDistance,
	float repelForce,
	float matchVelocityStrength,
	float centerForce,
	float maxVelocity
)
:	m_boidEntities(boidEntities)
,	m_transform(transform)
,	m_constrain(constrain)
,	m_followForce(followForce)
,	m_repelDistance(repelDistance)
,	m_repelForce(repelForce)
,	m_matchVelocityStrength(matchVelocityStrength)
,	m_centerForce(centerForce)
,	m_maxVelocity(maxVelocity)
{
	RandomGeometry random;

	m_boids.resize(m_boidEntities.size());
	for (AlignedVector< Boid >::iterator i = m_boids.begin(); i != m_boids.end(); ++i)
	{
		i->position = m_transform * (random.nextUnit() * spawnPositionDiagonal + Vector4(0.0f, 0.0f, 0.0f, 1.0f));
		i->velocity = random.nextUnit() * spawnVelocityDiagonal;
	}
}

BoidsEntity::~BoidsEntity()
{
}

void BoidsEntity::destroy()
{
	for (RefArray< world::Entity >::iterator i = m_boidEntities.begin(); i != m_boidEntities.end(); ++i)
	{
		if (*i)
			(*i)->destroy();
	}
	m_boidEntities.resize(0);
}

void BoidsEntity::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
	for (RefArray< world::Entity >::iterator i = m_boidEntities.begin(); i != m_boidEntities.end(); ++i)
	{
		if (*i)
			worldContext.build(worldRenderView, worldRenderPass, *i);
	}
}

void BoidsEntity::setTransform(const Transform& transform)
{
	m_transform = transform;
}

bool BoidsEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform;
	return true;
}

Aabb3 BoidsEntity::getBoundingBox() const
{
	Transform transformInv = m_transform.inverse();
	
	Aabb3 aabb;
	for (uint32_t i = 0; i < uint32_t(m_boids.size()); ++i)
		aabb.contain(transformInv * m_boids[i].position);

	return aabb;
}

void BoidsEntity::update(const world::UpdateParams& update)
{
	Scalar deltaTime(min(update.deltaTime, 1.0f / 30.0f));
	Vector4 attraction = m_transform.translation().xyz1();

	// Calculate perceived center and velocity of all boids.
	Vector4 center(0.0f, 0.0f, 0.0f, 0.0f);
	Vector4 velocity(0.0f, 0.0f, 0.0f, 0.0f);
	for (uint32_t i = 0; i < m_boids.size(); ++i)
	{
		center += m_boids[i].position;
		velocity += m_boids[i].velocity;
	}

	Vector4 constrainAdd = attraction * (Vector4::one() - m_constrain);

	Scalar invBoidsSize(1.0f / (float(m_boids.size()) - 1.0f));

	// Update boids.
	for (uint32_t i = 0; i < m_boids.size(); ++i)
	{
		Vector4 otherCenter = center - m_boids[i].position;
		Vector4 otherVelocity = velocity - m_boids[i].velocity;

		otherCenter *= invBoidsSize;
		otherVelocity *= invBoidsSize;

		// 1: Follow perceived center.
		m_boids[i].velocity += (otherCenter - m_boids[i].position) * Scalar(m_followForce);

		// 2: Keep distance from other boids.
		for (uint32_t j = 0; j < uint32_t(m_boids.size()); ++j)
		{
			if (i != j)
			{
				Vector4 d = m_boids[j].position - m_boids[i].position;
				if (d.normalize() < m_repelDistance)
					m_boids[i].velocity -= d * Scalar(m_repelForce);
			}
		}

		// 3: Try to match velocity with other boids.
		m_boids[i].velocity += (otherVelocity - m_boids[i].velocity) * Scalar(m_matchVelocityStrength);

		// 4: Always try to be circulating around center.
		m_boids[i].velocity += (attraction - m_boids[i].position).xyz0() * Scalar(m_centerForce);

		// 5: Clamp velocity.
		Scalar ln = m_boids[i].velocity.length();
		m_boids[i].velocity = m_boids[i].velocity.normalized() * min(ln, Scalar(m_maxVelocity));

		// Integrate position.
		m_boids[i].position += m_boids[i].velocity * deltaTime;

		// Constrain position.
		m_boids[i].position = m_boids[i].position * m_constrain + constrainAdd;

		// Update boid entity.
		if (m_boidEntities[i])
		{
			m_boidEntities[i]->setTransform(Transform(
				lookAt(m_boids[i].position, m_boids[i].position + m_boids[i].velocity).inverse()
			));
			m_boidEntities[i]->update(update);
		}
	}
}

	}
}
