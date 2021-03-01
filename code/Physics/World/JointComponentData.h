#pragma once

#include "Core/Ref.h"
#include "World/IEntityComponentData.h"

#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class PhysicsManager;
class JointComponent;
class JointDesc;

/*!
 * \ingroup Physics
 */
class T_DLLCLASS JointComponentData : public world::IEntityComponentData
{
	T_RTTI_CLASS;

public:
	JointComponentData() = default;

	explicit JointComponentData(JointDesc* jointDesc);

	Ref< JointComponent > createComponent(PhysicsManager* physicsManager) const;

	virtual void setTransform(const world::EntityData* owner, const Transform& transform) override final;

	virtual void serialize(ISerializer& s) override final;

private:
	Ref< JointDesc > m_jointDesc;
};

	}
}

