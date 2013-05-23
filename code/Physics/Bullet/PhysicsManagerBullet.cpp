#include <algorithm>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btConvexConvexAlgorithm.h>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/Save.h"
#include "Core/Thread/Acquire.h"
#include "Heightfield/Heightfield.h"
#include "Physics/AxisJointDesc.h"
#include "Physics/BallJointDesc.h"
#include "Physics/BoxShapeDesc.h"
#include "Physics/CapsuleShapeDesc.h"
#include "Physics/CollisionListener.h"
#include "Physics/ConeTwistJointDesc.h"
#include "Physics/CylinderShapeDesc.h"
#include "Physics/DynamicBodyDesc.h"
#include "Physics/HeightfieldShapeDesc.h"
#include "Physics/HingeJointDesc.h"
#include "Physics/Hinge2JointDesc.h"
#include "Physics/Mesh.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/SphereShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/Bullet/Conversion.h"
#include "Physics/Bullet/AxisJointBullet.h"
#include "Physics/Bullet/BallJointBullet.h"
#include "Physics/Bullet/BodyBullet.h"
#include "Physics/Bullet/ConeTwistJointBullet.h"
#include "Physics/Bullet/HeightfieldShapeBullet.h"
#include "Physics/Bullet/HingeJointBullet.h"
#include "Physics/Bullet/Hinge2JointBullet.h"
#include "Physics/Bullet/PhysicsManagerBullet.h"
#include "Resource/IResourceManager.h"

#if defined(T_BULLET_USE_SPURS)

#	include "Core/Thread/Ps3/Spurs/SpursManager.h"

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

namespace traktor
{
	namespace physics
	{
		namespace
		{

class MeshProxyIndexVertexArray : public btStridingMeshInterface
{
public:
	MeshProxyIndexVertexArray(const resource::Proxy< Mesh >& mesh)
	:	m_mesh(mesh)
	{
	}

	virtual ~MeshProxyIndexVertexArray()
	{
		m_mesh.clear();
	}

	virtual void getLockedVertexIndexBase(unsigned char **vertexbase, int& numverts,PHY_ScalarType& type, int& stride,unsigned char **indexbase,int & indexstride,int& numfaces,PHY_ScalarType& indicestype,int subpart=0)
	{
		const AlignedVector< Vector4 >& vertices = m_mesh->getVertices();
		const std::vector< Mesh::Triangle >& shapeTriangles = m_mesh->getShapeTriangles();

		numverts = int(vertices.size());
		(*vertexbase) = (unsigned char *)&vertices[0];
		type = PHY_FLOAT;
		stride = sizeof(Vector4);
		numfaces = int(shapeTriangles.size());
		(*indexbase) = (unsigned char *)&shapeTriangles[0];
		indexstride = sizeof(Mesh::Triangle);
		indicestype = PHY_INTEGER;
	}

	virtual void getLockedReadOnlyVertexIndexBase(const unsigned char **vertexbase, int& numverts,PHY_ScalarType& type, int& stride,const unsigned char **indexbase,int & indexstride,int& numfaces,PHY_ScalarType& indicestype,int subpart=0) const
	{
		const AlignedVector< Vector4 >& vertices = m_mesh->getVertices();
		const std::vector< Mesh::Triangle >& shapeTriangles = m_mesh->getShapeTriangles();

		numverts = int(vertices.size());
		(*vertexbase) = (const unsigned char *)&vertices[0];
		type = PHY_FLOAT;
		stride = sizeof(Vector4);
		numfaces = int(shapeTriangles.size());
		(*indexbase) = (const unsigned char *)&shapeTriangles[0];
		indexstride = sizeof(Mesh::Triangle);
		indicestype = PHY_INTEGER;
	}

	virtual void unLockVertexBase(int subpart)
	{
	}

	virtual void unLockReadOnlyVertexBase(int subpart) const
	{
	}

	virtual int getNumSubParts() const
	{
		return 1;
	}

	virtual void preallocateVertices(int numverts)
	{
	}

	virtual void preallocateIndices(int numindices)
	{
	}

private:
	resource::Proxy< Mesh > m_mesh;
};

uint32_t getCollisionGroup(const btCollisionObject* collisionObject)
{
	if (!collisionObject)
		return ~0UL;

	BodyBullet* body = static_cast< BodyBullet* >(collisionObject->getUserPointer());
	if (!body)
		return ~0UL;

	return body->getCollisionGroup();
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
	uint32_t m_queryTypes;
	btCollisionObject* m_excludeObject;

	ClosestRayExcludeResultCallback(btCollisionObject* excludeObject, uint32_t group, uint32_t queryTypes, const btVector3& rayFromWorld, const btVector3& rayToWorld)
	:	m_rayFromWorld(rayFromWorld)
	,	m_rayToWorld(rayToWorld)
	,	m_excludeObject(excludeObject)
	,	m_group(group)
	,	m_queryTypes(queryTypes)
	{
	}

	virtual	btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
	{
		T_ASSERT (rayResult.m_hitFraction <= m_closestHitFraction);
		
		if (m_excludeObject == rayResult.m_collisionObject)
			return m_closestHitFraction;

		if (m_group != ~0UL && (getCollisionGroup(rayResult.m_collisionObject) & m_group) == 0)
			return m_closestHitFraction;

		bool isStatic = rayResult.m_collisionObject->isStaticOrKinematicObject();
		if (
			( isStatic && (m_queryTypes & PhysicsManager::QtStatic ) != 0) ||
			(!isStatic && (m_queryTypes & PhysicsManager::QtDynamic) != 0)
		)
		{
			m_closestHitFraction = rayResult.m_hitFraction;
			m_collisionObject = rayResult.m_collisionObject;

			if (normalInWorldSpace)
				m_hitNormalWorld = rayResult.m_hitNormalLocal;
			else
				m_hitNormalWorld = m_collisionObject->getWorldTransform().getBasis() * rayResult.m_hitNormalLocal;

			m_hitPointWorld.setInterpolate3(m_rayFromWorld, m_rayToWorld, rayResult.m_hitFraction);
		}

		return m_closestHitFraction;
	}
};

struct ClosestRayExcludeAndCullResultCallback : public btCollisionWorld::RayResultCallback
{
	btVector3 m_rayFromWorld;
	btVector3 m_rayToWorld;
	btVector3 m_hitNormalWorld;
	btVector3 m_hitPointWorld;
	uint32_t m_group;
	btCollisionObject* m_excludeObject;

	ClosestRayExcludeAndCullResultCallback(btCollisionObject* excludeObject, uint32_t group, const btVector3& rayFromWorld, const btVector3& rayToWorld)
	:	m_rayFromWorld(rayFromWorld)
	,	m_rayToWorld(rayToWorld)
	,	m_excludeObject(excludeObject)
	,	m_group(group)
	{
	}

	virtual	btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
	{
		T_ASSERT (rayResult.m_hitFraction <= m_closestHitFraction);
		
		if (m_excludeObject == rayResult.m_collisionObject)
			return m_closestHitFraction;

		if (m_group != ~0UL && (getCollisionGroup(rayResult.m_collisionObject) & m_group) == 0)
			return m_closestHitFraction;

		btVector3 hitNormalWorld;
		if (normalInWorldSpace)
			hitNormalWorld = rayResult.m_hitNormalLocal;
		else
			hitNormalWorld = rayResult.m_collisionObject->getWorldTransform().getBasis() * rayResult.m_hitNormalLocal;

		if (hitNormalWorld.dot(m_rayToWorld - m_rayFromWorld) > 0.0f)
			return m_closestHitFraction;

		m_closestHitFraction = rayResult.m_hitFraction;
		m_collisionObject = rayResult.m_collisionObject;
		m_hitNormalWorld = hitNormalWorld;
		m_hitPointWorld.setInterpolate3(m_rayFromWorld, m_rayToWorld, rayResult.m_hitFraction);

		return m_closestHitFraction;
	}
};

struct ConvexExcludeResultCallback : public btCollisionWorld::ConvexResultCallback
{
	uint32_t m_group;
	btCollisionObject* m_excludeObject;
	RefArray< Body >& m_outResult;

	ConvexExcludeResultCallback(uint32_t group, btCollisionObject* excludeObject, RefArray< Body >& outResult)
	:	m_group(group)
	,	m_excludeObject(excludeObject)
	,	m_outResult(outResult)
	{
	}

	virtual	btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace)
	{
		if (m_excludeObject == convexResult.m_hitCollisionObject)
			return 1.0f;

		if (m_group != ~0UL && (getCollisionGroup(convexResult.m_hitCollisionObject) & m_group) == 0)
			return 1.0f;

		BodyBullet* bodyBullet = reinterpret_cast< BodyBullet* >(convexResult.m_hitCollisionObject->getUserPointer());
		T_ASSERT (bodyBullet);

		m_outResult.push_back(bodyBullet);
		return 1.0f;
	}
};

struct ContactResultCallback : public btCollisionWorld::ContactResultCallback
{
	const btCollisionObject* m_colObj;
	RefArray< Body >& m_outResult;

	ContactResultCallback(const btCollisionObject* colObj, RefArray< Body >& outResult)
	:	m_colObj(colObj)
	,	m_outResult(outResult)
	{
	}

#if 0
	virtual	btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0, int partId0, int index0, const btCollisionObjectWrapper* colObj1, int partId1, int index1)
	{
		if (m_colObj == colObj0->getCollisionObject())
		{
			T_ASSERT (colObj1);

			BodyBullet* bodyBullet = reinterpret_cast< BodyBullet* >(colObj1->getCollisionObject()->getUserPointer());
			T_ASSERT (bodyBullet);

			m_outResult.push_back(bodyBullet);
		}
		else if (m_colObj == colObj1->getCollisionObject())
		{
			T_ASSERT (colObj0);

			BodyBullet* bodyBullet = reinterpret_cast< BodyBullet* >(colObj0->getCollisionObject()->getUserPointer());
			T_ASSERT (bodyBullet);

			m_outResult.push_back(bodyBullet);
		}
		return 0.0f;
	}
#else
	virtual	btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObject* colObj0, int partId0, int index0, const btCollisionObject* colObj1, int partId1, int index1)
	{
		if (m_colObj == colObj0)
		{
			T_ASSERT (colObj1);

			BodyBullet* bodyBullet = reinterpret_cast< BodyBullet* >(colObj1->getUserPointer());
			T_ASSERT (bodyBullet);

			m_outResult.push_back(bodyBullet);
		}
		else if (m_colObj == colObj1)
		{
			T_ASSERT (colObj0);

			BodyBullet* bodyBullet = reinterpret_cast< BodyBullet* >(colObj0->getUserPointer());
			T_ASSERT (bodyBullet);

			m_outResult.push_back(bodyBullet);
		}
		return 0.0f;
	}
#endif
};

void deleteShape(btCollisionShape* shape)
{
	if (shape->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE)
	{
		btBvhTriangleMeshShape* triangleMeshShape = static_cast< btBvhTriangleMeshShape* >(shape);
		btStridingMeshInterface* stridingMesh = triangleMeshShape->getMeshInterface();
		delete stridingMesh;
	}
	else if (shape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE)
	{
		btCompoundShape* compoundShape = static_cast< btCompoundShape* >(shape);
		int numChildShapes = compoundShape->getNumChildShapes();
		for (int i = 0; i < numChildShapes; ++i)
			deleteShape(compoundShape->getChildShape(i));
	}
	delete shape;
}

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
	T_ANONYMOUS_VAR(RefArray< BodyBullet >)(m_bodies);

	while (!m_joints.empty())
		m_joints.front()->destroy();
	while (!m_bodies.empty())
		m_bodies.front()->destroy();

	m_joints.clear();
	m_bodies.clear();

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
	Vector4 centerOfGravity = Vector4::origo();
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
		shape = new btCylinderShapeZ(btVector3(radius, radius, length / 2.0f));
	}
	else if (const MeshShapeDesc* meshShape = dynamic_type_cast< const MeshShapeDesc* >(shapeDesc))
	{
		resource::Proxy< Mesh > mesh;
		if (!resourceManager->bind(meshShape->getMesh(), mesh))
		{
			log::error << L"Unable to load collision mesh resource " << Guid(meshShape->getMesh()).format() << Endl;
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
			hullPoints.reserve(hullIndices.size());
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
			MeshProxyIndexVertexArray* indexVertexArray = new MeshProxyIndexVertexArray(mesh);
			shape = new btBvhTriangleMeshShape(indexVertexArray, false);
		}

		centerOfGravity = mesh->getOffset();
	}
	else if (const SphereShapeDesc* sphereShape = dynamic_type_cast< const SphereShapeDesc* >(shapeDesc))
	{
		shape = new btSphereShape(sphereShape->getRadius());
	}
	else if (const HeightfieldShapeDesc* heightfieldShape = dynamic_type_cast< const HeightfieldShapeDesc* >(shapeDesc))
	{
		resource::Proxy< hf::Heightfield > heightfield;
		if (!resourceManager->bind(heightfieldShape->getHeightfield(), heightfield))
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
		Ref< BodyBullet > staticBody = new BodyBullet(
			this,
			m_dynamicsWorld,
			m_simulationDeltaTime,
			rigidBody,
			shape,
			centerOfGravity,
			shapeDesc->getCollisionGroup(),
			shapeDesc->getCollisionMask(),
			shapeDesc->getMaterial()
		);
		m_bodies.push_back(staticBody);

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
		Ref< BodyBullet > dynamicBody = new BodyBullet(
			this,
			m_dynamicsWorld,
			m_simulationDeltaTime,
			rigidBody,
			shape,
			centerOfGravity,
			shapeDesc->getCollisionGroup(),
			shapeDesc->getCollisionMask(),
			shapeDesc->getMaterial()
		);
		m_bodies.push_back(dynamicBody);

		rigidBody->setUserPointer(dynamicBody);
		body = dynamicBody;
	}
	else
	{
		log::error << L"Unsupported body type \"" << type_name(desc) << L"\"" << Endl;
		return 0;
	}

	// Ensure wrapper body is placed at origo in order
	// to set COG offset in Bullet body.
	if (body)
		body->setTransform(Transform::identity());

	return body;
}

Ref< Joint > PhysicsManagerBullet::createJoint(const JointDesc* desc, const Transform& transform, Body* body1, Body* body2)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!desc)
		return 0;

	BodyBullet* bb1 = checked_type_cast< BodyBullet* >(body1);
	BodyBullet* bb2 = checked_type_cast< BodyBullet* >(body2);

	btRigidBody* b1 = body1 ? bb1->getBtRigidBody() : 0;
	btRigidBody* b2 = body2 ? bb2->getBtRigidBody() : 0;
	
	Ref< Joint > joint;

	if (const AxisJointDesc* axisDesc = dynamic_type_cast< const AxisJointDesc* >(desc))
	{
		btHingeConstraint* hingeConstraint = 0;

		if (b1 && b2)
		{
			Vector4 anchor = transform * axisDesc->getAnchor().xyz1();
			Vector4 axis = transform * axisDesc->getAxis().xyz0().normalized();

			btVector3 anchorIn1 = toBtVector3(bb1->getBodyTransform().inverse() * anchor);
			btVector3 anchorIn2 = toBtVector3(bb2->getBodyTransform().inverse() * anchor);
			btVector3 axisIn1 = toBtVector3(bb1->getBodyTransform().inverse() * axis);
			btVector3 axisIn2 = toBtVector3(bb2->getBodyTransform().inverse() * axis);

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
			Vector4 anchor = transform * axisDesc->getAnchor().xyz1();
			Vector4 axis = transform * axisDesc->getAxis().xyz0().normalized();

			btVector3 anchorIn1 = toBtVector3(bb1->getBodyTransform().inverse() * anchor);
			btVector3 axisIn1 = toBtVector3(bb1->getBodyTransform().inverse() * axis);

			hingeConstraint = new btHingeConstraint(
				*b1,
				anchorIn1,
				axisIn1
			);
		}

		hingeConstraint->setAngularOnly(true);

		joint = new AxisJointBullet(this, hingeConstraint, bb1, bb2);
	}
	else if (const BallJointDesc* ballDesc = dynamic_type_cast< const BallJointDesc* >(desc))
	{
		btPoint2PointConstraint* pointConstraint = 0;

		if (b1 && b2)
		{
			Vector4 anchor = transform * ballDesc->getAnchor().xyz1();
			Vector4 anchorIn1 = bb1->getBodyTransform().inverse() * anchor;
			Vector4 anchorIn2 = bb2->getBodyTransform().inverse() * anchor;

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
			Vector4 anchorIn1 = bb1->getBodyTransform().inverse() * anchor;

			pointConstraint = new btPoint2PointConstraint(
				*b1,
				toBtVector3(anchorIn1)
			);
		}

		joint = new BallJointBullet(this, pointConstraint, bb1, bb2);
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
				bb1,
				bb2,
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
				bb1,
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
			Vector4 axis = transform * hingeDesc->getAxis().xyz0().normalized();

			btVector3 anchorIn1 = toBtVector3(bb1->getBodyTransform().inverse() * anchor);
			btVector3 anchorIn2 = toBtVector3(bb2->getBodyTransform().inverse() * anchor);
			btVector3 axisIn1 = toBtVector3(bb1->getBodyTransform().inverse() * axis);
			btVector3 axisIn2 = toBtVector3(bb2->getBodyTransform().inverse() * axis);

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

			btVector3 anchorIn1 = toBtVector3(bb1->getBodyTransform().inverse() * anchor);
			btVector3 axisIn1 = toBtVector3(bb1->getBodyTransform().inverse() * axis);

			hingeConstraint = new btHingeConstraint(
				*b1,
				anchorIn1,
				axisIn1
			);
		}

		if (hingeDesc->getEnableLimits())
		{
			float minAngle, maxAngle;
			hingeDesc->getAngles(minAngle, maxAngle);

			if (maxAngle - minAngle >= FUZZY_EPSILON)
				hingeConstraint->setLimit(minAngle, maxAngle, 1.0f);
			else
				hingeConstraint->setLimit(-0.001f, 0.001f, 1.0f);
		}

		hingeConstraint->setAngularOnly(hingeDesc->getAngularOnly());

		joint = new HingeJointBullet(this, hingeConstraint, bb1, bb2);
	}
	else if (const Hinge2JointDesc* hinge2Desc = dynamic_type_cast< const Hinge2JointDesc* >(desc))
	{
		btHinge2Constraint* hinge2Constraint = 0;

		if (b1 && b2)
		{
			Vector4 anchor = transform * hinge2Desc->getAnchor().xyz1();
			Vector4 axis1 = transform * hinge2Desc->getAxis1().xyz0().normalized();
			Vector4 axis2 = transform * hinge2Desc->getAxis2().xyz0().normalized();

			btVector3 _anchor = toBtVector3(anchor);
			btVector3 _axis1 = toBtVector3(axis1);
			btVector3 _axis2 = toBtVector3(axis2);

			hinge2Constraint = new btHinge2Constraint(
				*b1,
				*b2,
				_anchor,
				_axis1,
				_axis2
			);

			// Disable spring.
			if (hinge2Desc->getSuspensionEnable())
			{
				hinge2Constraint->enableSpring(2, true);
				hinge2Constraint->setDamping(2, hinge2Desc->getSuspensionDamping());
				hinge2Constraint->setStiffness(2, hinge2Desc->getSuspensionStiffness());
			}
			else
			{
				hinge2Constraint->enableSpring(2, false);
				hinge2Constraint->setDamping(2, 0.0f);
				hinge2Constraint->setStiffness(2, 0.0f);
			}

			// Setup rotation range allowed around axis1.
			hinge2Constraint->setLowerLimit(hinge2Desc->getLowStop());
			hinge2Constraint->setUpperLimit(hinge2Desc->getHighStop()); 

			hinge2Constraint->setEquilibriumPoint();
		}
		else
			return 0;

		joint = new Hinge2JointBullet(this, hinge2Constraint, bb1, bb2);
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

	//// Save current state of all dynamic bodies.
	//for (RefArray< DynamicBodyBullet >::iterator i = m_dynamicBodies.begin(); i != m_dynamicBodies.end(); ++i)
	//	(*i)->setPreviousState((*i)->getState());

	// Step simulation.
	m_dynamicsWorld->stepSimulation(m_simulationDeltaTime, 0);

	// Issue collision events.
	{
		CollisionInfo info;

		int32_t manifoldCount = m_dispatcher->getNumManifolds();
		for (int32_t i = 0; i < manifoldCount; ++i)
		{
			btPersistentManifold* manifold = m_dispatcher->getManifoldByIndexInternal(i);
			T_ASSERT (manifold);

			// Only call to listeners when a new manifold has been created.
			if (!manifold->m_fresh)
				continue;

			int32_t contacts = manifold->getNumContacts();
			if (contacts <= 0)
				continue;

			const btRigidBody* body0 = reinterpret_cast< const btRigidBody* >(manifold->getBody0());
			const btRigidBody* body1 = reinterpret_cast< const btRigidBody* >(manifold->getBody1());

			BodyBullet* wrapperBody0 = body0 ? static_cast< BodyBullet* >(body0->getUserPointer()) : 0;
			BodyBullet* wrapperBody1 = body1 ? static_cast< BodyBullet* >(body1->getUserPointer()) : 0;

			info.body1 = wrapperBody0;
			info.body2 = wrapperBody1;
			info.contacts.resize(0);
			info.contacts.reserve(contacts);

			for (int32_t j = 0; j < contacts; ++j)
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

			if (!info.contacts.empty())
			{
				notifyCollisionListeners(info);
				manifold->m_fresh = false;
			}
		}
	}
}

uint32_t PhysicsManagerBullet::getCollidingPairs(std::vector< CollisionPair >& outCollidingPairs) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

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
	bool ignoreBackFace,
	QueryResult& outResult
) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	btVector3 from = toBtVector3(at);
	btVector3 to = toBtVector3(at + direction * Scalar(maxLength));

	btRigidBody* excludeBody = ignoreBody ? checked_type_cast< const BodyBullet* >(ignoreBody)->getBtRigidBody() : 0;

	if (!ignoreBackFace)
	{
		ClosestRayExcludeResultCallback callback(excludeBody, group, QtAll, from, to);
		m_dynamicsWorld->rayTest(from, to, callback);
		if (!callback.hasHit())
			return false;

		BodyBullet* body = reinterpret_cast< BodyBullet* >(callback.m_collisionObject->getUserPointer());
		T_ASSERT (body);

		outResult.body = body;
		outResult.position = fromBtVector3(callback.m_hitPointWorld, 1.0f);
		outResult.normal = fromBtVector3(callback.m_hitNormalWorld, 0.0).normalized();
		outResult.distance = dot3(direction, outResult.position - at);
		outResult.material = body->getMaterial(); 
	}
	else
	{
		ClosestRayExcludeAndCullResultCallback callback(excludeBody, group, from, to);
		m_dynamicsWorld->rayTest(from, to, callback);
		if (!callback.hasHit())
			return false;

		BodyBullet* body = reinterpret_cast< BodyBullet* >(callback.m_collisionObject->getUserPointer());
		T_ASSERT (body);

		outResult.body = body;
		outResult.position = fromBtVector3(callback.m_hitPointWorld, 1.0f);
		outResult.normal = fromBtVector3(callback.m_hitNormalWorld, 0.0).normalized();
		outResult.distance = dot3(direction, outResult.position - at);
		outResult.material = body->getMaterial();
	}

	return true;
}

bool PhysicsManagerBullet::queryShadowRay(
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	uint32_t group,
	uint32_t queryTypes,
	const Body* ignoreBody
) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	btVector3 from = toBtVector3(at);
	btVector3 to = toBtVector3(at + direction * Scalar(maxLength));

	btRigidBody* excludeBody = ignoreBody ? checked_type_cast< const BodyBullet* >(ignoreBody)->getBtRigidBody() : 0;

	ClosestRayExcludeResultCallback callback(excludeBody, group, queryTypes, from, to);
	m_dynamicsWorld->rayTest(from, to, callback);
	if (!callback.hasHit())
		return false;

	return true;
}

uint32_t PhysicsManagerBullet::querySphere(
	const Vector4& at,
	float radius,
	uint32_t group,
	uint32_t queryTypes,
	RefArray< Body >& outBodies
) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	outBodies.resize(0);
	for (RefArray< BodyBullet >::const_iterator i = m_bodies.begin(); i != m_bodies.end(); ++i)
	{
		if (((*i)->getCollisionGroup() & group) == 0)
			continue;

		bool st = (*i)->isStatic();
		if ((queryTypes & QtStatic) == 0 && st)
			continue;
		if ((queryTypes & QtDynamic) == 0 && !st)
			continue;

		btRigidBody* rigidBody = (*i)->getBtRigidBody();
		T_ASSERT (rigidBody);

		btVector3 aabbMin, aabbMax;
		rigidBody->getAabb(aabbMin, aabbMax);

		float bodyRadius = (aabbMax - aabbMin).length() * 0.5f;
		Vector4 bodyCenter = fromBtVector3((aabbMin + aabbMax) * 0.5f, 1.0f);

		if ((bodyCenter - at).length() - radius - bodyRadius <= 0.0f)
			outBodies.push_back(*i);
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
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	btSphereShape sphereShape(radius);
	btTransform from, to;

	from.setIdentity();
	from.setOrigin(toBtVector3(at));

	to.setIdentity();
	to.setOrigin(toBtVector3(at + direction * Scalar(maxLength)));

	btRigidBody* excludeBody = ignoreBody ? checked_type_cast< const BodyBullet* >(ignoreBody)->getBtRigidBody() : 0;

	ClosestConvexExcludeResultCallback callback(group, excludeBody, from.getOrigin(), to.getOrigin());
	m_dynamicsWorld->convexSweepTest(
		&sphereShape,
		from,
		to,
		callback
	);
	if (!callback.hasHit())
		return false;

	BodyBullet* body = reinterpret_cast< BodyBullet* >(callback.m_hitCollisionObject->getUserPointer());
	T_ASSERT (body);

	outResult.body = body;
	outResult.position = fromBtVector3(callback.m_hitPointWorld, 1.0f);
	outResult.normal = fromBtVector3(callback.m_hitNormalWorld, 0.0).normalized();
	outResult.distance = dot3(direction, outResult.position - at);
	outResult.fraction = callback.m_closestHitFraction;
	outResult.material = body->getMaterial();

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
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	btRigidBody* rigidBody = checked_type_cast< const BodyBullet* >(body)->getBtRigidBody();
	btCollisionShape* shape = rigidBody->getCollisionShape();

	// If shape is a compound we assume it's first child is a convex shape.
	btQuaternion localRotation(0.0f, 0.0f, 0.0f, 1.0f);
	if (shape->isCompound())
	{
		btCompoundShape* compoundShape = static_cast< btCompoundShape* >(shape);
		if (compoundShape->getNumChildShapes() > 0)
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

	ClosestConvexExcludeResultCallback callback(group, rigidBody/*excludeBody*/, from.getOrigin(), to.getOrigin());
	m_dynamicsWorld->convexSweepTest(
		static_cast< const btConvexShape* >(shape),
		from,
		to,
		callback
	);
	if (!callback.hasHit())
		return false;

	BodyBullet* bodyBullet = reinterpret_cast< BodyBullet* >(callback.m_hitCollisionObject->getUserPointer());
	T_ASSERT (bodyBullet);

	outResult.body = bodyBullet;
	outResult.position = fromBtVector3(callback.m_hitPointWorld, 1.0f);
	outResult.normal = fromBtVector3(callback.m_hitNormalWorld, 0.0).normalized();
	outResult.distance = dot3(direction, outResult.position - at);
	outResult.fraction = callback.m_closestHitFraction;
	outResult.material = bodyBullet->getMaterial();

	return true;
}

void PhysicsManagerBullet::querySweep(
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	float radius,
	uint32_t group,
	const Body* ignoreBody,
	RefArray< Body >& outResult
) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	btSphereShape sphereShape(radius);
	btTransform from, to;

	from.setIdentity();
	from.setOrigin(toBtVector3(at));

	to.setIdentity();
	to.setOrigin(toBtVector3(at + direction * Scalar(maxLength)));

	ConvexExcludeResultCallback callback(
		group,
		ignoreBody ? checked_type_cast< const BodyBullet* >(ignoreBody)->getBtRigidBody() : 0,
		outResult
	);
	m_dynamicsWorld->convexSweepTest(
		&sphereShape,
		from,
		to,
		callback
	);
}

void PhysicsManagerBullet::queryOverlap(
	const Body* body,
	RefArray< Body >& outResult
) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	btRigidBody* rigidBody = checked_type_cast< const BodyBullet* >(body)->getBtRigidBody();
	T_ASSERT (rigidBody);

	ContactResultCallback callback(rigidBody, outResult);
	m_dynamicsWorld->contactTest(rigidBody, callback);
}

void PhysicsManagerBullet::getBodyCount(uint32_t& outCount, uint32_t& outActiveCount) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	const btCollisionObjectArray& collisionObjects = m_dynamicsWorld->getCollisionObjectArray();
	for (int i = 0; i < collisionObjects.size(); ++i)
	{
		++outCount;
		if (collisionObjects[i]->isActive())
			++outActiveCount;
	}
}

void PhysicsManagerBullet::insertBody(btRigidBody* rigidBody, uint16_t collisionGroup, uint16_t collisionFilter)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_dynamicsWorld->addRigidBody(rigidBody, collisionGroup, collisionFilter);
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

void PhysicsManagerBullet::destroyBody(BodyBullet* body, btRigidBody* rigidBody, btCollisionShape* shape)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	m_dynamicsWorld->removeRigidBody(rigidBody);

	RefArray< BodyBullet >::iterator i = std::find(m_bodies.begin(), m_bodies.end(), body);
	T_ASSERT (i != m_bodies.end());
	m_bodies.erase(i);

	delete rigidBody;
	deleteShape(shape);
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

	BodyBullet* body1 = colObj0 ? static_cast< BodyBullet* >(colObj0->getUserPointer()) : 0;
	BodyBullet* body2 = colObj1 ? static_cast< BodyBullet* >(colObj1->getUserPointer()) : 0;
	if (body1 && body2)
	{
		// Filter on cluster id.
		uint32_t clusterId1 = body1->getClusterId();
		if (clusterId1 != ~0UL && clusterId1 == body2->getClusterId())
			return;

		// Filter collision on collision group and mask first.
		uint32_t group1 = body1->getCollisionGroup();
		uint32_t mask1 = body1->getCollisionMask();

		uint32_t group2 = body2->getCollisionGroup();
		uint32_t mask2 = body2->getCollisionMask();

		if ((group1 & mask2) == 0 && (group2 & mask1) == 0)
			return;

		// Skip bodies which are directly connected through a joint.
		const std::vector< Joint* >& joints = body1->getJoints();
		for (std::vector< Joint* >::const_iterator i = joints.begin(); i != joints.end(); ++i)
		{
			if ((*i)->getBody1() == body2 || (*i)->getBody2() == body2)
				return;
		}
	}

	btCollisionDispatcher::defaultNearCallback(collisionPair, dispatcher, dispatchInfo);
}

	}
}
