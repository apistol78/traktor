/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_CharacterComponent_H
#define traktor_physics_CharacterComponent_H

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

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void setOwner(world::Entity* owner) T_OVERRIDE T_FINAL;

	virtual void setTransform(const Transform& transform) T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual void update(const world::UpdateParams& update) T_OVERRIDE T_FINAL;

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

#endif	// traktor_physics_CharacterComponent_H
