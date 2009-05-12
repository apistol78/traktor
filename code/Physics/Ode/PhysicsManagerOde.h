#ifndef traktor_physics_PhysicsManagerOde_H
#define traktor_physics_PhysicsManagerOde_H

#include "Core/Heap/Ref.h"
#include "Physics/PhysicsManager.h"

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

class PhysicsManagerOdeImpl;

/*!
 * \ingroup ODE
 */
class T_DLLCLASS PhysicsManagerOde : public PhysicsManager
{
	T_RTTI_CLASS(PhysicsManagerOde)

public:
	struct Settings
	{
		float simulationDeltaTime;	//< Simulation delta time.
		float globalErp;			//< Error reduction parameter.
		float globalCfm;			//< Global constraint force mixing.
		int maxContacts;			//< Maximum number of contacts.
		bool fastStepEnable;		//< Use fast simulation method.
		int fastStepIterations;

		Settings()
		:	globalErp(0.2f)
		,	globalCfm(1e-5f)
		,	maxContacts(4)
		,	fastStepEnable(true)
		,	fastStepIterations(10)
		{
		}
	};

	PhysicsManagerOde();

	virtual ~PhysicsManagerOde();

	bool create(const Settings& settings);

	virtual bool create(float simulationDeltaTime);

	virtual void destroy();

	virtual void setGravity(const Vector4& gravity);

	virtual Body* createBody(const BodyDesc* desc);

	virtual Joint* createJoint(const JointDesc* desc, const Matrix44& transform, Body* body1, Body* body2);

	virtual void update();

	virtual uint32_t getCollidingPairs(std::vector< CollisionPair >& outCollidingPairs) const;

	virtual bool queryPoint(const Vector4& at, float margin, QueryResult& outResult) const;

	virtual bool queryRay(const Vector4& at, const Vector4& direction, float maxLength, const Body* ignoreBody, QueryResult& outResult) const;

	virtual uint32_t querySphere(const Vector4& at, float radius, uint32_t queryTypes, RefArray< Body >& outBodies) const;

	virtual bool querySweep(const Vector4& at, const Vector4& direction, float maxLength, float radius, const Body* ignoreBody, QueryResult& outResult) const;

	virtual void getBodyCount(uint32_t& outCount, uint32_t& outActiveCount) const;

private:
	PhysicsManagerOdeImpl* m_impl;
};

	}
}

#endif	// traktor_physics_PhysicsManagerOde_H
