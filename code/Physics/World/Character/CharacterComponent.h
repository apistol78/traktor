/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "World/IEntityComponent.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::physics
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
	explicit CharacterComponent(
		PhysicsManager* physicsManager,
		const CharacterComponentData* data,
		Body* bodyWide,
		Body* bodySlim,
		uint32_t traceInclude,
		uint32_t traceIgnore
	);

	virtual ~CharacterComponent();

	virtual void destroy() override final;

	virtual void setOwner(world::Entity* owner) override final;

	virtual void setTransform(const Transform& transform) override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void update(const world::UpdateParams& update) override final;

	void setHeadAngle(float headAngle);

	float getHeadAngle() const;

	/*! Dead stop.
	 */
	void stop();

	/*! Clear accumulated impulses.
	 *
	 * Impulses are automatically cleared after
	 * each update, this method is only for
	 * filtering "pending" impulses within update.
	 */
	void clear();

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

	/*!*/
	void setVelocity(const Vector4& velocity);

	/*! Character current velocity. */
	const Vector4& getVelocity() const;

	/*!*/
	void setMaxVelocity(float maxVelocity) { m_maxVelocity = maxVelocity; }

	/*!*/
	float getMaxVelocity() const { return m_maxVelocity; }

	/*! Enable or disable the character controller.
	 *
	 * When disabled the controller stops simulating entirely (no gravity,
	 * stepping or owner transform updates) and its collision body is removed
	 * from the world, so another system - such as a rag doll - can take over
	 * the entity without the controller fighting it.
	 */
	void setEnable(bool enable);

	/*! Return true if the character controller is enabled. */
	bool isEnable() const { return m_enabled; }

private:
	world::Entity* m_owner;
	Ref< PhysicsManager > m_physicsManager;
	Ref< const CharacterComponentData > m_data;
	Ref< Body > m_bodyWide;
	Ref< Body > m_bodySlim;
	uint32_t m_traceInclude;
	uint32_t m_traceIgnore;
	float m_maxVelocity;
	float m_headAngle;
	Vector4 m_velocity;
	Vector4 m_impulse;
	bool m_grounded;
	bool m_enabled;

	bool stepVertical(float motion, Vector4& inoutPosition) const;

	bool step(Vector4 motion, Vector4& inoutPosition) const;
};

}
