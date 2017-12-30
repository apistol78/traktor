/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_physics_CharacterComponentData_H
#define traktor_physics_CharacterComponentData_H

#include <set>
#include "Resource/Id.h"
#include "World/IEntityComponentData.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace resource
	{

class IResourceManager;

	}

	namespace world
	{

class Entity;
class IEntityBuilder;

	}

	namespace physics
	{

class BodyDesc;
class CollisionSpecification;
class PhysicsManager;
class CharacterComponent;

/*! \brief
 * \ingroup Physics
 */
class T_DLLCLASS CharacterComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	CharacterComponentData();

	explicit CharacterComponentData(BodyDesc* bodyDesc);

	Ref< CharacterComponent > createComponent(
		const world::IEntityBuilder* entityBuilder,
		resource::IResourceManager* resourceManager,
		PhysicsManager* physicsManager
	) const;

	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

	const BodyDesc* getBodyDesc() const { return m_bodyDesc; }

	const std::set< resource::Id< CollisionSpecification > >& getTraceInclude() const { return m_traceInclude; }

	const std::set< resource::Id< CollisionSpecification > >& getTraceIgnore() const { return m_traceIgnore; }

	float getStepHeight() const { return m_stepHeight; }

private:
	Ref< BodyDesc > m_bodyDesc;
	std::set< resource::Id< CollisionSpecification > > m_traceInclude;
	std::set< resource::Id< CollisionSpecification > > m_traceIgnore;
	float m_stepHeight;
};

	}
}

#endif	// traktor_physics_CharacterComponentData_H
