#ifndef traktor_physics_DynamicBodyOde_H
#define traktor_physics_DynamicBodyOde_H

#include <ode/ode.h>
#include "Physics/DynamicBody.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_ODE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

struct DestroyCallback;

/*!
 * \ingroup ODE
 */
class T_DLLCLASS DynamicBodyOde : public DynamicBody
{
	T_RTTI_CLASS(DynamicBodyOde)

public:
	DynamicBodyOde(
		DestroyCallback* callback,
		dWorldID worldId,
		dJointGroupID contactGroupId,
		dTriMeshDataID meshDataId,
		dGeomID geomId,
		dBodyID bodyId
	);

	virtual ~DynamicBodyOde();

	virtual void destroy();

	virtual void setTransform(const Matrix44& transform);

	virtual Matrix44 getTransform() const;

	virtual void reset();

	virtual void setMass(float mass, const Vector4& inertiaTensor);

	virtual float getInverseMass() const;

	virtual Matrix33 getInertiaTensorInverseWorld() const;

	virtual void addForceAt(const Vector4& at, const Vector4& force, bool localSpace);

	virtual void addTorque(const Vector4& torque, bool localSpace);

	virtual void addLinearImpulse(const Vector4& linearImpulse, bool localSpace);

	virtual void addAngularImpulse(const Vector4& angularImpulse, bool localSpace);

	virtual void addImpulse(const Vector4& at, const Vector4& impulse, bool localSpace);

	virtual void setLinearVelocity(const Vector4& linearVelocity);

	virtual Vector4 getLinearVelocity() const;

	virtual void setAngularVelocity(const Vector4& angularVelocity);

	virtual Vector4 getAngularVelocity() const;

	virtual Vector4 getVelocityAt(const Vector4& at, bool localSpace) const;

	virtual bool setState(const DynamicBodyState& state);

	virtual DynamicBodyState getState() const;

	virtual void setActive(bool active);

	virtual bool isActive() const;

	virtual void setEnable(bool enable);

	virtual bool isEnable() const;

	const dGeomID getGeomId() const;

	const dBodyID getBodyId() const;

private:
	DestroyCallback* m_callback;
	dWorldID m_worldId;
	dJointGroupID m_contactGroupId;
	dTriMeshDataID m_meshDataId;
	dGeomID m_geomId;
	dBodyID m_bodyId;
};

	}
}

#endif	// traktor_physics_DynamicBodyOde_H
