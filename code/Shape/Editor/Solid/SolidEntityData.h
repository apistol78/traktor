#pragma once

#include <set>
#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Resource/Id.h"
#include "World/Entity/GroupEntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
    namespace physics
    {

class CollisionSpecification;

    }

	namespace render
	{

class IRenderSystem;
class Shader;

	}

    namespace resource
    {

class IResourceManager;

    }

    namespace world
    {

class IEntityBuilder;

    }

    namespace shape
    {

class PrimitiveEntityData;
class SolidEntity;

/*! Solid geometry entity data.
 *
 * Contain solid geometry primitives along with operation (intersection, union, difference etc).
 */
class T_DLLCLASS SolidEntityData : public world::GroupEntityData
{
    T_RTTI_CLASS;

public:
    Ref< SolidEntity > createEntity(const world::IEntityBuilder* builder, resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem) const;

    virtual void serialize(ISerializer& s) override final;

    const resource::Id< render::Shader >& getShader() const { return m_shader; }

	void setCollisionGroup(const std::set< resource::Id< physics::CollisionSpecification > >& collisionGroup);

	const std::set< resource::Id< physics::CollisionSpecification > >& getCollisionGroup() const;

	void setCollisionMask(const std::set< resource::Id< physics::CollisionSpecification > >& collisionMask);

	const std::set< resource::Id< physics::CollisionSpecification > >& getCollisionMask() const;

private:
    resource::Id< render::Shader > m_shader;
	std::set< resource::Id< physics::CollisionSpecification > > m_collisionGroup;
	std::set< resource::Id< physics::CollisionSpecification > > m_collisionMask;
};

    }
}