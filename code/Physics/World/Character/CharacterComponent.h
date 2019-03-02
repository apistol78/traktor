#pragma once

#include "World/IEntityComponent.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace world
	{

class Entity;

	}

	namespace physics
	{

class Body;
class CharacterComponentData;
class PhysicsManager;

/*! \brief
 * \ingroup Physics
 */
class T_DLLCLASS CharacterComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	CharacterComponent(
		PhysicsManager* physicsManager,
		const CharacterComponentData* data,
		Body* body,
		uint32_t traceInclude,
		uint32_t traceIgnore
	);

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	void setHeadAngle(float headAngle);

	float getHeadAngle() const;

	void setVelocity(const Vector4& velocity);

	const Vector4& getVelocity() const;

	bool isGrounded() const;

private:
	world::Entity* m_owner;
	Ref< PhysicsManager > m_physicsManager;
	Ref< const CharacterComponentData > m_data;
	Ref< Body > m_body;
	uint32_t m_traceInclude;
	uint32_t m_traceIgnore;
	float m_headAngle;
	Vector4 m_velocity;
	bool m_grounded;
};

	}
}

