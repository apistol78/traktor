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

/*! Character component.
 * \ingroup Physics
 */
class T_DLLCLASS CharacterComponent : public world::IEntityComponent
{
	T_RTTI_CLASS;

public:
	CharacterComponent(
		PhysicsManager* physicsManager,
		const CharacterComponentData* data,
		Body* bodyWide,
		Body* bodySlim,
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

	/*! Move character.
	 *
	 * \param motion New velocity of character.
	 * \param vertical If vertical velocity should be set.
	 */
	void move(const Vector4& motion, bool vertical);

	/*! Issue character jump.
	 *
	 * Only issued if character is grounded using
	 * the impulse specified in the character data.
	 *
	 * \return True if jump has been initiated.
	 */
	bool jump();

	/*! Return true if character is grounded. */
	bool grounded() const;

	/*! Character current velocity. */
	const Vector4& getVelocity() const;

private:
	world::Entity* m_owner;
	Ref< PhysicsManager > m_physicsManager;
	Ref< const CharacterComponentData > m_data;
	Ref< Body > m_bodyWide;
	Ref< Body > m_bodySlim;
	uint32_t m_traceInclude;
	uint32_t m_traceIgnore;
	float m_headAngle;
	Vector4 m_velocity;
	bool m_grounded;

	bool stepVertical(float motion, Vector4& inoutPosition) const;

	bool step(Vector4 motion, Vector4& inoutPosition) const;
};

	}
}

