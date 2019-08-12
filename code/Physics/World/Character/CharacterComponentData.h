#pragma once

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

class CharacterComponent;
class CollisionSpecification;
class PhysicsManager;
class ShapeDesc;

/*! \brief
 * \ingroup Physics
 */
class T_DLLCLASS CharacterComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	CharacterComponentData();

	Ref< CharacterComponent > createComponent(
		const world::IEntityBuilder* entityBuilder,
		resource::IResourceManager* resourceManager,
		PhysicsManager* physicsManager
	) const;

	Ref< ShapeDesc > getShapeDesc(float epsilon) const;

	const std::set< resource::Id< CollisionSpecification > >& getTraceInclude() const { return m_traceInclude; }

	const std::set< resource::Id< CollisionSpecification > >& getTraceIgnore() const { return m_traceIgnore; }

	float getRadius() const { return m_radius; }

	float getHeight() const { return m_height; }

	float getStep() const { return m_step; }

	virtual void serialize(ISerializer& s) override final;

private:
	std::set< resource::Id< CollisionSpecification > > m_traceInclude;
	std::set< resource::Id< CollisionSpecification > > m_traceIgnore;
	float m_radius;
	float m_height;
	float m_step;
};

	}
}

