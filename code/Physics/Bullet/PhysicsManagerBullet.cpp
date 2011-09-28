#include <algorithm>

#if defined(_PS3)
#	define T_BULLET_USE_SPURS
#endif

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btConvexConvexAlgorithm.h>

#if defined(T_BULLET_USE_SPURS)

// Collision detection.
#	include <SpuDispatch/BulletCollisionSpursSupport.h>
#	include <BulletMultiThreaded/SpuGatheringCollisionDispatcher.h>

// Constraint solver.
#	include <SpuDispatch/BulletPE2ConstraintSolverSpursSupport.h>
#	include <BulletMultiThreaded/btParallelConstraintSolver.h>
#	include <BulletCollision/CollisionDispatch/btSimulationIslandManager.h>

// Integration.
#	include <SpuDispatch/btParallelDynamicsWorld.h>
#	include <SpuDispatch/BulletPEGatherScatterSpursSupport.h>

#endif

#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/Save.h"
#include "Core/Thread/Acquire.h"

#if defined(T_BULLET_USE_SPURS)
#	include "Core/Thread/Ps3/Spurs/SpursManager.h"
#endif

#include "Physics/CollisionListener.h"
#include "Physics/BoxShapeDesc.h"
#include "Physics/CapsuleShapeDesc.h"
#include "Physics/CylinderShapeDesc.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/SphereShapeDesc.h"
#include "Physics/HeightfieldShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/DynamicBodyDesc.h"
#include "Physics/BallJointDesc.h"
#include "Physics/ConeTwistJointDesc.h"
#include "Physics/HingeJointDesc.h"
#include "Physics/Hinge2JointDesc.h"
#include "Physics/Mesh.h"
#include "Physics/Heightfield.h"
#include "Physics/Bullet/PhysicsManagerBullet.h"
#include "Physics/Bullet/DynamicBodyBullet.h"
#include "Physics/Bullet/StaticBodyBullet.h"
#include "Physics/Bullet/BallJointBullet.h"
#include "Physics/Bullet/ConeTwistJointBullet.h"
#include "Physics/Bullet/HingeJointBullet.h"
#include "Physics/Bullet/HeightfieldShapeBullet.h"
#include "Physics/Bullet/Conversion.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace physics
	{
		namespace
		{

uint32_t getCollisionGroup(const btCollisionObject* collisionObject)
{
	if (!collisionObject)
		return ~0UL;

	Body* body = static_cast< Body* >(collisionObject->getUserPointer());
	if (!body)
		return ~0UL;

	if (DynamicBodyBullet* dynamicBody = dynamic_type_cast< DynamicBodyBullet* >(body))
		return dynamicBody->getCollisionGroup();
	if (StaticBodyBullet* staticBody = dynamic_type_cast< StaticBodyBullet* >(body))
		return staticBody->getCollisionGroup();

	return ~0UL;
}

struct ClosestConvexExcludeResultCallback : public btCollisionWorld::ClosestConvexResultCallback
{
	uint32_t m_group;
	btCollisionObject* m_excludeObject;

	ClosestConvexExcludeResultCallback(uint32_t group, btCollisionObject* excludeObject, const btVector3& convexFromWorld, const btVector3& convexToWorld)
	:	btCollisionWorld::ClosestConvexResultCallback(convexFromWorld, convexToWorld)
	,	m_group(group)
	,	m_excludeObject(excludeObject)
	{
	}

	virtual	btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace)
	{
		T_ASSERT (convexResult.m_hitFraction <= m_closestHitFraction);

		if (m_excludeObject == convexResult.m_hitCollisionObject)
			return 1.0f;

		if (m_group != ~0UL && (getCollisionGroup(convexResult.m_hitCollisionObject) & m_group) == 0)
			return 1.0f;
		
		return btCollisionWorld::ClosestConvexResultCallback::addSingleResult(convexResult, normalInWorldSpace);
	}
};

struct ClosestRayExcludeResultCallback : public btCollisionWorld::RayResultCallback
{
	btVector3 m_rayFromWorld;
	btVector3 m_rayToWorld;
	btVector3 m_hitNormalWorld;
	btVector3 m_hitPointWorld;
	uint32_t m_group;
	btCollisionObject* m_excludeObject;
	uint32_t m_collisionPart;

	ClosestRayExcludeResultCallback(btCollisionObject* excludeObject, uint32_t group, const btVector3& rayFromWorld, const btVector3& rayToWorld)
	:	m_rayFromWorld(rayFromWorld)
	,	m_rayToWorld(rayToWorld)
	,	m_excludeObject(excludeObject)
	,	m_group(group)
	,	m_collisionPart(0)
	{
	}

	virtual	btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
	{
		T_ASSERT (rayResult.m_hitFraction <= m_closestHitFraction);
		
		if (m_excludeObject == rayResult.m_collisionObject)
			return m_closestHitFraction;

		if (m_group != ~0UL && (getCollisionGroup(rayResult.m_collisionObject) & m_group) == 0)
			return m_closestHitFraction;

		m_closestHitFraction = rayResult.m_hitFraction;
		m_collisionObject = rayResult.m_collisionObject;
		m_collisionPart = rayResult.m_localShapeInfo ? rayResult.m_localShapeInfo->m_shapePart : 0;

		if (normalInWorldSpace)
			m_hitNormalWorld = rayResult.m_hitNormalLocal;
		else
			m_hitNormalWorld = m_collisionObject->getWorldTransform().getBasis() * rayResult.m_hitNormalLocal;

		m_hitPointWorld.setInterpolate3(m_rayFromWorld, m_rayToWorld, rayResult.m_hitFraction);

		return m_closestHitFraction;
	}
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.PhysicsManagerBullet", 0, PhysicsManagerBullet, PhysicsManager)

PhysicsManagerBullet* PhysicsManagerBullet::ms_this = 0;

PhysicsManagerBullet::PhysicsManagerBullet()
:	m_simulationDeltaTime(0.0f)
,	m_configuration(0)
,	m_dispatcher(0)
,	m_broadphase(0)
,	m_solver(0)
,	m_dynamicsWorld(0)
{
}

PhysicsManagerBullet::~PhysicsManagerBullet()
{
	T_ASSERT (!m_configuration);
	T_ASSERT (!m_dispatcher);
	T_ASSERT (!m_broadphase);
	T_ASSERT (!m_solver);
	T_ASSERT (!m_dynamicsWorld);
}

bool PhysicsManagerBullet::create(float simulationDeltaTime)
{
	btDefaultCollisionConstructionInfo info;
	info.m_defaultStackAllocatorSize = 512 * 1024;

	m_simulationDeltaTime = simulationDeltaTime;
	m_configuration = new btDefaultCollisionConfiguration(info);
	
#if !defined(T_BULLET_USE_SPURS)
	m_dispatcher = new btCollisionDispatcher(m_configuration);
	m_solver = new btSequentialImpulseConstraintSolver();
	m_broadphase = new btDbvtBroadphase();
	m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_configuration);
#else
	btThreadSupportInterface* collisionThreadSupport = new BulletCollisionSpursSupport(
		SpursManager::getInstance().getSpurs(),
		SpursManager::getInstance().getSpuCount(),
		SpursManager::getInstance().getSpuCount()
	);

	m_dispatcher = new SpuGatheringCollisionDispatcher(
		collisionThreadSupport,
		SpursManager::getInstance().getSpuCount(),
		m_configuration
	);

#	if 1
	btThreadSupportInterface* constraintThreadSupport = new BulletPE2ConstraintSolverSpursSupport(
		SpursManager::getInstance().getSpurs(),
		SpursManager::getInstance().getSpuCount(),
		SpursManager::getInstance().getSpuCount()
	);
	m_solver = new btParallelConstraintSolver(constraintThreadSupport);
#	else
	m_solver = new btSequentialImpulseConstraintSolver();
#	endif

	m_broadphase = new btDbvtBroadphase();

#	if 1
	btThreadSupportInterface* integrateThreadSupport = new BulletPEGatherScatterSpursSupport(
		SpursManager::getInstance().getSpurs(),
		SpursManager::getInstance().getSpuCount(),
		SpursManager::getInstance().getSpuCount()
	);

	m_dynamicsWorld = new btParallelDynamicsWorld(
		m_dispatcher,
		m_broadphase,
		m_solver,
		m_configuration,
		integrateThreadSupport
	);
#	else
	m_dynamicsWorld = new btDiscreteDynamicsWorld(
		m_dispatcher,
		m_broadphase,
		m_solver,
		m_configuration
	);
#	endif

#endif

	m_dispatcher->setNearCallback(&PhysicsManagerBullet::nearCallback);

	return true;
}

void PhysicsManagerBullet::destroy()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ANONYMOUS_VAR(RefArray< Joint >)(m_joints);
	T_ANONYMOUS_VAR(RefArray< StaticBodyBullet >)(m_staticBodies);
	T_ANONYMOUS_VAR(RefArray< DynamicBodyBullet >)(m_dynamicBodies);

	while (!m_joints.empty())
		m_joints.front()->destroy();
	while (!m_dynamicBodies.empty())
		m_dynamicBodies.front()->destroy();
	while (!m_staticBodies.empty())
		m_staticBodies.front()->destroy();

	m_joints.clear();
	m_dynamicBodies.clear();
	m_staticBodies.clear();

	delete m_dynamicsWorld; m_dynamicsWorld = 0;
	delete m_solver; m_solver = 0;
	delete m_broadphase; m_broadphase = 0;
	delete m_dispatcher; m_dispatcher = 0;
	delete m_configuration; m_configuration = 0;
}

void PhysicsManagerBullet::setGravity(const Vector4& gravity)
{
	T_ASSERT (m_dynamicsWorld);
	m_dynamicsWorld->setGravity(toBtVector3(gravity));
}

Vector4 PhysicsManagerBullet::getGravity() const
{
	T_ASSERT (m_dynamicsWorld);
	return fromBtVector3(m_dynamicsWorld->getGravity(), 0.0f);
}

Ref< Body > PhysicsManagerBullet::createBody(resource::IResourceManager* resourceManager, const BodyDesc* desc)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!desc)
		return 0;

	const ShapeDesc* shapeDesc = desc->getShape();
	if (!shapeDesc)
	{
		log::error << L"Unable to create body, no shape defined" << Endl;
		return 0;
	}

	// Create collision shape.
	btCollisionShape* shape = 0;

	if (const BoxShapeDesc* boxShape = dynamic_type_cast< const BoxShapeDesc* >(shapeDesc))
	{
		shape = new btBoxShape(toBtVector3(boxShape->getExtent()));
	}
	else if (const CapsuleShapeDesc* capsuleShape = dynamic_type_cast< const CapsuleShapeDesc* >(shapeDesc))
	{
		float radius = capsuleShape->getRadius();
		float length = capsuleShape->getLength() - radius * 2.0f;
		T_ASSERT (length >= 0.0f);
		shape = new btCapsuleShapeZ(radius, length);
	}
	else if (const CylinderShapeDesc* cylinderShape = dynamic_type_cast< const CylinderShapeDesc* >(shapeDesc))
	{
		float radius = cylinderShape->getRadius();
		float length = cylinderShape->getLength();
		shape = new btCylinderShapeZ(btVector3(radius, radius, length));
	}
	else if (const MeshShapeDesc* meshShape = dynamic_type_cast< const MeshShapeDesc* >(shapeDesc))
	{
		resource::Proxy< Mesh > mesh = meshShape->getMesh();
		if (!resourceManager->bind(mesh) || !mesh.validate())
		{
			log::error << L"Unable to load mesh resource" << Endl;
			return 0;
		}

		const AlignedVector< Vector4 >& vertices = mesh->getVertices();
		const std::vector< Mesh::Triangle >& shapeTriangles = mesh->getShapeTriangles();
		const std::vector< Mesh::Triangle >& hullTriangles = mesh->getHullTriangles();

		if (is_a< DynamicBodyDesc >(desc))
		{
			if (hullTriangles.empty())
			{
				log::error << L"Unable to create body, mesh hull empty" << Endl;
				return 0;
			}

			// Extract hull points.
			std::set< uint32_t > hullIndices;
			for (std::vector< Mesh::Triangle >::const_iterator i = hullTriangles.begin(); i != hullTriangles.end(); ++i)
			{
				hullIndices.insert(i->indices[0]);
				hullIndices.insert(i->indices[1]);
				hullIndices.insert(i->indices[2]);
			}

			// Build point list, only hull points.
			AlignedVector< Vector4 > hullPoints;
			for (std::set< uint32_t >::iterator j = hullIndices.begin(); j != hullIndices.end(); ++j)
				hullPoints.push_back(vertices[*j]);

			// Create Bullet shape.
			shape = new btConvexHullShape(
				static_cast< const btScalar* >(reinterpret_cast< const float* >(&hullPoints[0])),
				int(hullPoints.size()),
				sizeof(Vector4)
			);
		}
		else
		{
			btTriangleIndexVertexArray* indexVertexArray = new btTriangleIndexVertexArray(
				int(shapeTriangles.size()),
				const_cast< int * >(reinterpret_cast< const int* >(&shapeTriangles[0])),
				sizeof(Mesh::Triangle),
				int(vertices.size()),
				const_cast< btScalar* >(reinterpret_cast< const float* >(&vertices[0])),
				sizeof(Vector4)
			);

			shape = new btBvhTriangleMeshShape(indexVertexArray, false);
		}
	}
	else if (const SphereShapeDesc* sphereShape = dynamic_type_cast< const SphereShapeDesc* >(shapeDesc))
	{
		shape = new btSphereShape(sphereShape->getRadius());
	}
	else if (const HeightfieldShapeDesc* heightfieldShape = dynamic_type_cast< const HeightfieldShapeDesc* >(shapeDesc))
	{
		resource::Proxy< Heightfield > heightfield = heightfieldShape->getHeightfield();
		if (!resourceManager->bind(heightfield) || !heightfield.validate())
		{
			log::error << L"Unable to load heightfield resource" << Endl;
			return 0;
		}

		shape = new HeightfieldShapeBullet(heightfield);
	}
	else
	{
		log::error << L"Unsupported shape type \"" << type_name(shapeDesc) << L"\"" << Endl;
		return 0;
	}

	T_ASSERT (shape);

	// Create compound shape which has the shape's local transformation.
	if (shapeDesc->getLocalTransform() != Transform::identity())
	{
		btCompoundShape* compound = new btCompoundShape();
		compound->addChildShape(toBtTransform(shapeDesc->getLocalTransform()), shape);
		shape = compound;
	}

	// Create rigid body.
	Ref< Body > body;

	if (const StaticBodyDesc* staticDesc = dynamic_type_cast< const StaticBodyDesc* >(desc))
	{
		// Create body.
		btRigidBody::btRigidBodyConstructionInfo info(0.0f, new btDefaultMotionState(), shape);
		info.m_friction = staticDesc->getFriction();

		btRigidBody* rigidBody = new btRigidBody(info);

		// Set static flag.
		rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);

		// Define body as kinematic.
		if (staticDesc->isKinematic())
		{
			rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
			rigidBody->setActivationState(DISABLE_DEACTIVATION);
		}

		// Create our wrapper.
		Ref< StaticBodyBullet > staticBody = new StaticBodyBullet(
			this,
			m_dynamicsWorld,
			rigidBody,
			shape,
			shapeDesc->getCollisionGroup(),
			shapeDesc->getCollisionMask()
		);
		m_staticBodies.push_back(staticBody);

		rigidBody->setUserPointer(staticBody);
		body = staticBody;
	}
	else if (const DynamicBodyDesc* dynamicDesc = dynamic_type_cast< const DynamicBodyDesc* >(desc))
	{
		float mass = dynamicDesc->getMass();

		// Calculate inertia from shape.
		btVector3 localInertia(0.0f, 0.0f, 0.0f);
		shape->calculateLocalInertia(mass, localInertia);

		// Create body.
		btRigidBody::btRigidBodyConstructionInfo info(mass, new btDefaultMotionState(), shape, localInertia);
		info.m_linearDamping = dynamicDesc->getLinearDamping();
		info.m_angularDamping = dynamicDesc->getAngularDamping();
		info.m_friction = dynamicDesc->getFriction();
		info.m_linearSleepingThreshold = dynamicDesc->getLinearThreshold();
		info.m_angularSleepingThreshold = dynamicDesc->getAngularThreshold();
		btRigidBody* rigidBody = new btRigidBody(info);

		if (!dynamicDesc->getActive())
		{
			T_ASSERT_M (dynamicDesc->getAutoDeactivate(), L"If body is initially disabled then auto deactivate must be set as well");
			rigidBody->forceActivationState(ISLAND_SLEEPING);
		}
		else
		{
			if (!dynamicDesc->getAutoDeactivate())
				rigidBody->forceActivationState(DISABLE_DEACTIVATION);
			else
				rigidBody->forceActivationState(ACTIVE_TAG);
		}

		// Create our wrapper.
		Ref< DynamicBodyBullet > dynamicBody = new DynamicBodyBullet(
			this,
			m_dynamicsWorld,
			rigidBody,
			shape,
			shapeDesc->getCollisionGroup(),
			shapeDesc->getCollisionMask()
		);
		m_dynamicBodies.push_back(dynamicBody);

		rigidBody->setUserPointer(dynamicBody);
		body = dynamicBody;
	}
	else
	{
		log::error << L"Unsupported body type \"" << type_name(desc) << L"\"" << Endl;
		return 0;
	}

	return body;
}

Ref< Joint > PhysicsManagerBullet::createJoint(const JointDesc* desc, const Transform& transform, Body* body1, Body* body2)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!desc)
		return 0;

	btRigidBody* b1 = 0;
	btRigidBody* b2 = 0;

	if (DynamicBodyBullet* dynamicBody1 = dynamic_type_cast< DynamicBodyBullet* >(body1))
		b1 = dynamicBody1->getBtRigidBody();
	else if (StaticBodyBullet* staticBody1 = dynamic_type_cast< StaticBodyBullet* >(body1))
		b1 = staticBody1->getBtRigidBody();

	T_ASSERT (b1);

	if (body2)
	{
		if (DynamicBodyBullet* dynamicBody2 = dynamic_type_cast< DynamicBodyBullet* >(body2))
			b2 = dynamicBody2->getBtRigidBody();
		else if (StaticBodyBullet* staticBody2 = dynamic_type_cast< StaticBodyBullet* >(body2))
			b2 = staticBody2->getBtRigidBody();

		T_ASSERT (b2);
	}
	
	Ref< Joint > joint;

	if (const BallJointDesc* ballDesc = dynamic_type_cast< const BallJointDesc* >(desc))
	{
		btPoint2PointConstraint* pointConstraint = 0;

		if (b1 && b2)
		{
			Vector4 anchor = transform * ballDesc->getAnchor().xyz1();
			Vector4 anchorIn1 = body1->getTransform().inverse() * anchor;
			Vector4 anchorIn2 = body2->getTransform().inverse() * anchor;

			pointConstraint = new btPoint2PointConstraint(
				*b1,
				*b2,
				toBtVector3(anchorIn1),
				toBtVector3(anchorIn2)
			);
		}
		else
		{
			Vector4 anchor = transform * ballDesc->getAnchor().xyz1();
			Vector4 anchorIn1 = body1->getTransform().inverse() * anchor;

			pointConstraint = new btPoint2PointConstraint(
				*b1,
				toBtVector3(anchorIn1)
			);
		}

		joint = new BallJointBullet(this, pointConstraint, body1, body2);
	}
	else if (const ConeTwistJointDesc* coneTwistDesc = dynamic_type_cast< const ConeTwistJointDesc* >(desc))
	{
		JointConstraint* jointConstraint;
		
		if (b1 && b2)
		{
			jointConstraint = new JointConstraint(*b1, *b2);

			Ref< ConeTwistJointBullet > coneTwistJoint = new ConeTwistJointBullet(
				this,
				jointConstraint,
				transform,
				body1,
				body2,
				coneTwistDesc
			);

			jointConstraint->setJointSolver(coneTwistJoint);
			joint = coneTwistJoint;
		}
		else
		{
			jointConstraint = new JointConstraint(*b1);

			Ref< ConeTwistJointBullet > coneTwistJoint = new ConeTwistJointBullet(
				this,
				jointConstraint,
				transform,
				body1,
				coneTwistDesc
			);

			jointConstraint->setJointSolver(coneTwistJoint);
			joint = coneTwistJoint;
		}
	}
	else if (const HingeJointDesc* hingeDesc = dynamic_type_cast< const HingeJointDesc* >(desc))
	{
		btHingeConstraint* hingeConstraint = 0;

		if (b1 && b2)
		{
			Vector4 anchor = transform * hingeDesc->getAnchor().xyz1();
			Vector4 axis = transform * hingeDesc->getAxis().xyz0();

			btVector3 anchorIn1 = toBtVector3(body1->getTransform().inverse() * anchor);
			btVector3 anchorIn2 = toBtVector3(body2->getTransform().inverse() * anchor);
			btVector3 axisIn1 = toBtVector3(body1->getTransform().inverse() * axis);
			btVector3 axisIn2 = toBtVector3(body2->getTransform().inverse() * axis);

			hingeConstraint = new btHingeConstraint(
				*b1,
				*b2,
				anchorIn1,
				anchorIn2,
				axisIn1,
				axisIn2
			);
		}
		else
		{
			Vector4 anchor = transform * hingeDesc->getAnchor().xyz1();
			Vector4 axis = transform * hingeDesc->getAxis().xyz0();

			btVector3 anchorIn1 = toBtVector3(body1->getTransform().inverse() * anchor);
			btVector3 axisIn1 = toBtVector3(body1->getTransform().inverse() * axis);

			hingeConstraint = new btHingeConstraint(
				*b1,
				anchorIn1,
				axisIn1
			);
		}

		float minAngle, maxAngle;
		hingeDesc->getAngles(minAngle, maxAngle);
		if (abs(maxAngle - minAngle) > FUZZY_EPSILON)
			hingeConstraint->setLimit(minAngle, maxAngle);

		joint = new HingeJointBullet(this, hingeConstraint, body1, body2);
	}

	if (!joint)
	{
		log::error << L"Unable to create joint, unknown joint type \"" << type_name(desc) << L"\"" << Endl;
		return 0;
	}

	m_joints.push_back(joint);
	return joint;
}

void PhysicsManagerBullet::update()
{
	T_ASSERT (m_dynamicsWorld);

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ANONYMOUS_VAR(Save< PhysicsManagerBullet* >)(ms_this, this);

	// Save current state of all dynamic bodies.
	for (RefArray< DynamicBodyBullet >::iterator i = m_dynamicBodies.begin(); i != m_dynamicBodies.end(); ++i)
		(*i)->setPreviousState((*i)->getState());

	// Step simulation.
	m_dynamicsWorld->stepSimulation(m_simulationDeltaTime, 0);

	// Issue collision events.
	int manifoldCount = m_dispatcher->getNumManifolds();
	for (int i = 0; i < manifoldCount; ++i)
	{
		btPersistentManifold* manifold = m_dispatcher->getManifoldByIndexInternal(i);
		T_ASSERT (manifold);

		// Only call to listeners when a new manifold has been created.
		if (!manifold->m_fresh)
			continue;

		const btRigidBody* body0 = reinterpret_cast< const btRigidBody* >(manifold->getBody0());
		const btRigidBody* body1 = reinterpret_cast< const btRigidBody* >(manifold->getBody1());

		Body* wrapperBody0 = body0 ? static_cast< Body* >(body0->getUserPointer()) : 0;
		Body* wrapperBody1 = body1 ? static_cast< Body* >(body1->getUserPointer()) : 0;

		// Don't issue collision events between joined bodies.
		if (wrapperBody0 && wrapperBody1)
		{
			for (RefArray< Joint >::const_iterator i = m_joints.begin(); i != m_joints.end(); ++i)
			{
				if (
					((*i)->getBody1() == wrapperBody0 && (*i)->getBody2() == wrapperBody1) ||
					((*i)->getBody1() == wrapperBody1 && (*i)->getBody2() == wrapperBody0)
				)
					return;
			}
		}

		CollisionInfo info;
		info.body1 = wrapperBody0;
		info.body2 = wrapperBody1;

		int contacts = manifold->getNumContacts();
		for (int j = 0; j < contacts; ++j)
		{
			const btManifoldPoint& pt = manifold->getContactPoint(j);
			if (pt.getDistance() < 0.0f)
			{
				CollisionContact cc;
				cc.depth = -pt.getDistance();
				cc.normal = fromBtVector3(pt.m_normalWorldOnB, 0.0f);
				cc.position = fromBtVector3(pt.m_positionWorldOnA, 1.0f);
				info.contacts.push_back(cc);
			}
		}
		if (info.contacts.empty())
			continue;

		notifyCollisionListeners(info);
		manifold->m_fresh = false;
	}
}

uint32_t PhysicsManagerBullet::getCollidingPairs(std::vector< CollisionPair >& outCollidingPairs) const
{
	int manifoldCount = m_dispatcher->getNumManifolds();

	outCollidingPairs.reserve(manifoldCount);
	for (int i = 0; i < manifoldCount; ++i)
	{
		const btPersistentManifold* manifold = m_dispatcher->getManifoldByIndexInternal(i);
		T_ASSERT (manifold);

		bool validContact = false;
		int contacts = manifold->getNumContacts();
		for (int j = 0; j < contacts; ++j)
		{
			const btManifoldPoint& pt = manifold->getContactPoint(j);
			if (pt.getDistance() < 0.0f)
			{
				validContact = true;
				break;
			}
		}
		if (!validContact)
			continue;

		const btRigidBody* body0 = reinterpret_cast< const btRigidBody* >(manifold->getBody0());
		const btRigidBody* body1 = reinterpret_cast< const btRigidBody* >(manifold->getBody1());

		Body* wrapperBody0 = body0 ? static_cast< Body* >(body0->getUserPointer()) : 0;
		Body* wrapperBody1 = body1 ? static_cast< Body* >(body1->getUserPointer()) : 0;

		CollisionPair pair = { wrapperBody0, wrapperBody1 };
		outCollidingPairs.push_back(pair);
	}

	return manifoldCount;
}

bool PhysicsManagerBullet::queryPoint(const Vector4& at, float margin, QueryResult& outResult) const
{
	return false;
}

bool PhysicsManagerBullet::queryRay(
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	uint32_t group,
	const Body* ignoreBody,
	QueryResult& outResult
) const
{
	btVector3 from = toBtVector3(at);
	btVector3 to = toBtVector3(at + direction * Scalar(maxLength));

	btRigidBody* excludeBody = 0;

	if (const DynamicBodyBullet* dynamicBody = dynamic_type_cast< const DynamicBodyBullet* >(ignoreBody))
		excludeBody = dynamicBody->getBtRigidBody();
	else if (const StaticBodyBullet* staticBody = dynamic_type_cast< const StaticBodyBullet* >(ignoreBody))
		excludeBody = staticBody->getBtRigidBody();

	ClosestRayExcludeResultCallback callback(excludeBody, group, from, to);
	m_dynamicsWorld->rayTest(from, to, callback);
	if (!callback.hasHit())
		return false;

	outResult.body = callback.m_collisionObject ? reinterpret_cast< Body* >(callback.m_collisionObject->getUserPointer()) : 0;
	outResult.position = fromBtVector3(callback.m_hitPointWorld, 1.0f);
	outResult.normal = fromBtVector3(callback.m_hitNormalWorld, 0.0).normalized();
	outResult.distance = dot3(direction, outResult.position - at);
	outResult.part = callback.m_collisionPart;

	return true;
}

uint32_t PhysicsManagerBullet::querySphere(const Vector4& at, float radius, uint32_t queryTypes, RefArray< Body >& outBodies) const
{
	outBodies.resize(0);

	if (queryTypes & QtStatic)
	{
		for (RefArray< StaticBodyBullet >::const_iterator i = m_staticBodies.begin(); i != m_staticBodies.end(); ++i)
		{
			btRigidBody* rigidBody = (*i)->getBtRigidBody();
			T_ASSERT (rigidBody);

			btVector3 aabbMin, aabbMax;
			rigidBody->getAabb(aabbMin, aabbMax);

			float bodyRadius = (aabbMax - aabbMin).length() * 0.5f;
			Vector4 bodyCenter = fromBtVector3((aabbMin + aabbMax) * 0.5f, 1.0f);

			if ((bodyCenter - at).length() - radius - bodyRadius <= 0.0f)
				outBodies.push_back(*i);
		}
	}

	if (queryTypes & QtDynamic)
	{
		for (RefArray< DynamicBodyBullet >::const_iterator i = m_dynamicBodies.begin(); i != m_dynamicBodies.end(); ++i)
		{
			btRigidBody* rigidBody = (*i)->getBtRigidBody();
			T_ASSERT (rigidBody);

			btVector3 aabbMin, aabbMax;
			rigidBody->getAabb(aabbMin, aabbMax);

			float bodyRadius = (aabbMax - aabbMin).length() * 0.5f;
			Vector4 bodyCenter = fromBtVector3((aabbMin + aabbMax) * 0.5f, 1.0f);

			if ((bodyCenter - at).length() - radius - bodyRadius <= 0.0f)
				outBodies.push_back(*i);
		}
	}

	return uint32_t(outBodies.size());
}

bool PhysicsManagerBullet::querySweep(
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	float radius,
	uint32_t group,
	const Body* ignoreBody,
	QueryResult& outResult
) const
{
	btSphereShape sphereShape(radius);
	btTransform from, to;

	from.setIdentity();
	from.setOrigin(toBtVector3(at));

	to.setIdentity();
	to.setOrigin(toBtVector3(at + direction * Scalar(maxLength)));

	btRigidBody* excludeBody = 0;

	if (const DynamicBodyBullet* dynamicBody = dynamic_type_cast< const DynamicBodyBullet* >(ignoreBody))
		excludeBody = dynamicBody->getBtRigidBody();
	else if (const StaticBodyBullet* staticBody = dynamic_type_cast< const StaticBodyBullet* >(ignoreBody))
		excludeBody = staticBody->getBtRigidBody();

	ClosestConvexExcludeResultCallback callback(group, excludeBody, from.getOrigin(), to.getOrigin());
	m_dynamicsWorld->convexSweepTest(
		&sphereShape,
		from,
		to,
		callback
	);
	if (!callback.hasHit())
		return false;

	outResult.body = callback.m_hitCollisionObject ? reinterpret_cast< Body* >(callback.m_hitCollisionObject->getUserPointer()) : 0;
	outResult.position = fromBtVector3(callback.m_hitPointWorld, 1.0f);
	outResult.normal = fromBtVector3(callback.m_hitNormalWorld, 0.0).normalized();
	outResult.distance = dot3(direction, outResult.position - at);
	outResult.fraction = callback.m_closestHitFraction;
	outResult.part = 0;

	return true;
}

bool PhysicsManagerBullet::querySweep(
	const Body* body,
	const Quaternion& orientation,
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	uint32_t group,
	const Body* ignoreBody,
	QueryResult& outResult
) const
{
	btRigidBody* rigidBody = 0;
	btCollisionShape* shape = 0;

	if (const DynamicBodyBullet* dynamicBody = dynamic_type_cast< const DynamicBodyBullet* >(body))
		rigidBody = dynamicBody->getBtRigidBody();
	else if (const StaticBodyBullet* staticBody = dynamic_type_cast< const StaticBodyBullet* >(body))
		rigidBody = staticBody->getBtRigidBody();
	T_ASSERT (rigidBody);

	shape = rigidBody->getCollisionShape();
	T_ASSERT (shape);

	// If shape is a compound we assume it's first child is a convex shape.
	btQuaternion localRotation(0.0f, 0.0f, 0.0f, 1.0f);
	if (shape->isCompound())
	{
		btCompoundShape* compoundShape = static_cast< btCompoundShape* >(shape);
		shape = compoundShape->getChildShape(0);
		localRotation = compoundShape->getChildTransform(0).getRotation();
	}

	// Ensure shape is a convex shape; required when performing sweep test.
	if (!shape->isConvex())
		return false;

	btTransform from, to;

	from.setIdentity();
	from.setRotation(toBtQuaternion(orientation) * localRotation);
	from.setOrigin(toBtVector3(at));

	to.setIdentity();
	to.setRotation(toBtQuaternion(orientation) * localRotation);
	to.setOrigin(toBtVector3(at + direction * Scalar(maxLength)));

	//btRigidBody* excludeBody = 0;

	//if (const DynamicBodyBullet* dynamicBody = dynamic_type_cast< const DynamicBodyBullet* >(ignoreBody))
	//	excludeBody = dynamicBody->getBtRigidBody();
	//else if (const StaticBodyBullet* staticBody = dynamic_type_cast< const StaticBodyBullet* >(ignoreBody))
	//	excludeBody = staticBody->getBtRigidBody();

	ClosestConvexExcludeResultCallback callback(group, rigidBody/*excludeBody*/, from.getOrigin(), to.getOrigin());
	m_dynamicsWorld->convexSweepTest(
		static_cast< const btConvexShape* >(shape),
		from,
		to,
		callback
	);
	if (!callback.hasHit())
		return false;

	outResult.body = callback.m_hitCollisionObject ? reinterpret_cast< Body* >(callback.m_hitCollisionObject->getUserPointer()) : 0;
	outResult.position = fromBtVector3(callback.m_hitPointWorld, 1.0f);
	outResult.normal = fromBtVector3(callback.m_hitNormalWorld, 0.0).normalized();
	outResult.distance = dot3(direction, outResult.position - at);
	outResult.fraction = callback.m_closestHitFraction;
	outResult.part = 0;

	return true;
}

void PhysicsManagerBullet::getBodyCount(uint32_t& outCount, uint32_t& outActiveCount) const
{
	const btCollisionObjectArray& collisionObjects = m_dynamicsWorld->getCollisionObjectArray();
	for (int i = 0; i < collisionObjects.size(); ++i)
	{
		++outCount;
		if (collisionObjects[i]->isActive())
			++outActiveCount;
	}
}

void PhysicsManagerBullet::insertBody(btRigidBody* rigidBody)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_dynamicsWorld->addRigidBody(rigidBody);
}

void PhysicsManagerBullet::removeBody(btRigidBody* rigidBody)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_dynamicsWorld->removeRigidBody(rigidBody);
}

void PhysicsManagerBullet::insertConstraint(btTypedConstraint* constraint)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_dynamicsWorld->addConstraint(constraint);
}

void PhysicsManagerBullet::removeConstraint(btTypedConstraint* constraint)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_dynamicsWorld->removeConstraint(constraint);
}

void PhysicsManagerBullet::destroyBody(Body* body, btRigidBody* rigidBody, btCollisionShape* shape)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	m_dynamicsWorld->removeRigidBody(rigidBody);

	if (StaticBodyBullet* staticBody = dynamic_type_cast< StaticBodyBullet* >(body))
	{
		RefArray< StaticBodyBullet >::iterator i = std::find(m_staticBodies.begin(), m_staticBodies.end(), staticBody);
		T_ASSERT (i != m_staticBodies.end());
		m_staticBodies.erase(i);
	}
	else if (DynamicBodyBullet* dynamicBody = dynamic_type_cast< DynamicBodyBullet* >(body))
	{
		RefArray< DynamicBodyBullet >::iterator i = std::find(m_dynamicBodies.begin(), m_dynamicBodies.end(), dynamicBody);
		T_ASSERT(i != m_dynamicBodies.end());
		m_dynamicBodies.erase(i);
	}

	delete rigidBody;
	delete shape;
}

void PhysicsManagerBullet::destroyConstraint(Joint* joint, btTypedConstraint* constraint)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	m_dynamicsWorld->removeConstraint(constraint);

	RefArray< Joint >::iterator i = std::find(m_joints.begin(), m_joints.end(), joint);
	T_ASSERT(i != m_joints.end());
	m_joints.erase(i);

	delete constraint;
}

void PhysicsManagerBullet::nearCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo)
{
	T_ASSERT (ms_this);

	btCollisionObject* colObj0 = static_cast< btCollisionObject* >(collisionPair.m_pProxy0->m_clientObject);
	btCollisionObject* colObj1 = static_cast< btCollisionObject* >(collisionPair.m_pProxy1->m_clientObject);

	Body* body1 = colObj0 ? static_cast< Body* >(colObj0->getUserPointer()) : 0;
	Body* body2 = colObj1 ? static_cast< Body* >(colObj1->getUserPointer()) : 0;
	if (body1 && body2)
	{
		uint32_t group1 = 0, group2 = 0;
		uint32_t mask1 = 0, mask2 = 0;

		if (DynamicBodyBullet* dynamicBody1 = dynamic_type_cast< DynamicBodyBullet* >(body1))
		{
			group1 = dynamicBody1->getCollisionGroup();
			mask1 = dynamicBody1->getCollisionMask();
		}
		else if (StaticBodyBullet* staticBody1 = dynamic_type_cast< StaticBodyBullet* >(body1))
		{
			group1 = staticBody1->getCollisionGroup();
			mask1 = staticBody1->getCollisionMask();
		}

		if (DynamicBodyBullet* dynamicBody2 = dynamic_type_cast< DynamicBodyBullet* >(body2))
		{
			group2 = dynamicBody2->getCollisionGroup();
			mask2 = dynamicBody2->getCollisionMask();
		}
		else if (StaticBodyBullet* staticBody2 = dynamic_type_cast< StaticBodyBullet* >(body2))
		{
			group2 = staticBody2->getCollisionGroup();
			mask2 = staticBody2->getCollisionMask();
		}

		if ((group1 & mask2) == 0 && (group2 & mask1) == 0)
			return;

		// Skip bodies which are directly connected through a joint.
		// \fixme Doesn't scale whatsoever..
		for (RefArray< Joint >::const_iterator i = ms_this->m_joints.begin(); i != ms_this->m_joints.end(); ++i)
		{
			if (
				((*i)->getBody1() == body1 && (*i)->getBody2() == body2) ||
				((*i)->getBody1() == body2 && (*i)->getBody2() == body1)
			)
				return;
		}
	}

	btCollisionDispatcher::defaultNearCallback(collisionPair, dispatcher, dispatchInfo);
}

	}
}
