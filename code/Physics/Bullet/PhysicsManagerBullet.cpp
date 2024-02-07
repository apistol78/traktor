/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btConvexConvexAlgorithm.h>
#include <BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Math/Format.h"
#include "Core/Misc/Save.h"
#include "Core/Thread/Acquire.h"
#include "Heightfield/Heightfield.h"
#include "Physics/AxisJointDesc.h"
#include "Physics/BallJointDesc.h"
#include "Physics/BoxShapeDesc.h"
#include "Physics/CapsuleShapeDesc.h"
#include "Physics/CollisionListener.h"
#include "Physics/CollisionSpecification.h"
#include "Physics/CompoundShapeDesc.h"
#include "Physics/ConeTwistJointDesc.h"
#include "Physics/CylinderShapeDesc.h"
#include "Physics/DofJointDesc.h"
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
#include "Physics/Bullet/DofJointBullet.h"
#include "Physics/Bullet/HeightfieldShapeBullet.h"
#include "Physics/Bullet/HingeJointBullet.h"
#include "Physics/Bullet/Hinge2JointBullet.h"
#include "Physics/Bullet/PhysicsManagerBullet.h"
#include "Resource/IResourceManager.h"

namespace traktor::physics
{
	namespace
	{

void* traktorAlloc(size_t size)
{
	return getAllocator()->alloc(size, 16, "Bullet");
}

void* traktorAllocAlign(size_t size, int alignment)
{
	return getAllocator()->alloc(size, max(16, alignment), "Bullet");
}

void traktorFree(void* memblock)
{
	getAllocator()->free(memblock);
}

bool traktorContactAdded(
	btManifoldPoint& cp,
	const btCollisionObjectWrapper* colObj0Wrap,
	int partId0,
	int index0,
	const btCollisionObjectWrapper* colObj1Wrap,
	int partId1,
	int index1
)
{
	float friction[] = { 0.0f, 0.0f };
	float restitution[] = { 0.0f, 0.0f };

	const BodyBullet* body0 = static_cast< const BodyBullet* >(colObj0Wrap->getCollisionObject()->getUserPointer());
	body0->getFrictionAndRestitution(index0, friction[0], restitution[0]);

	const BodyBullet* body1 = static_cast< const BodyBullet* >(colObj1Wrap->getCollisionObject()->getUserPointer());
	body1->getFrictionAndRestitution(index1, friction[1], restitution[1]);

	cp.m_combinedFriction = friction[0] * friction[1];
	cp.m_combinedRestitution = restitution[0] * restitution[1];
	return true;
}

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

	virtual void getLockedVertexIndexBase(unsigned char** vertexbase, int& numverts,PHY_ScalarType& type, int& stride, unsigned char**indexbase, int& indexstride, int& numfaces, PHY_ScalarType& indicestype, int subpart = 0) override final
	{
		const AlignedVector< Vector4 >& vertices = m_mesh->getVertices();
		const AlignedVector< Mesh::Triangle >& shapeTriangles = m_mesh->getShapeTriangles();

		numverts = (int)vertices.size();
		(*vertexbase) = (unsigned char *)&vertices[0];
		type = PHY_FLOAT;
		stride = sizeof(Vector4);
		numfaces = (int)shapeTriangles.size();
		(*indexbase) = (unsigned char *)&shapeTriangles[0];
		indexstride = sizeof(Mesh::Triangle);
		indicestype = PHY_INTEGER;
	}

	virtual void getLockedReadOnlyVertexIndexBase(const unsigned char** vertexbase, int& numverts, PHY_ScalarType& type, int& stride, const unsigned char** indexbase, int & indexstride, int& numfaces, PHY_ScalarType& indicestype, int subpart = 0) const override final
	{
		const AlignedVector< Vector4 >& vertices = m_mesh->getVertices();
		const AlignedVector< Mesh::Triangle >& shapeTriangles = m_mesh->getShapeTriangles();

		numverts = (int)vertices.size();
		(*vertexbase) = (const unsigned char *)&vertices[0];
		type = PHY_FLOAT;
		stride = sizeof(Vector4);
		numfaces = (int)shapeTriangles.size();
		(*indexbase) = (const unsigned char *)&shapeTriangles[0];
		indexstride = sizeof(Mesh::Triangle);
		indicestype = PHY_INTEGER;
	}

	virtual void unLockVertexBase(int subpart) override final
	{
	}

	virtual void unLockReadOnlyVertexBase(int subpart) const override final
	{
	}

	virtual int getNumSubParts() const override final
	{
		return 1;
	}

	virtual void preallocateVertices(int numverts) override final
	{
	}

	virtual void preallocateIndices(int numindices) override final
	{
	}

	bool getTriangleNormal(int triangleIndex, Vector4& outHitNormal) const
	{
		const AlignedVector< Vector4 >& normals = m_mesh->getNormals();
		if (triangleIndex >= 0 && triangleIndex < normals.size())
		{
			outHitNormal = normals[triangleIndex];
			return true;
		}
		else
			return false;
	}

private:
	resource::Proxy< Mesh > m_mesh;
};

uint32_t getCollisionGroup(const btCollisionObject* collisionObject)
{
	if (!collisionObject)
		return ~0U;

	BodyBullet* body = static_cast< BodyBullet* >(collisionObject->getUserPointer());
	if (!body)
		return ~0U;

	return body->getCollisionGroup();
}

uint32_t getClusterId(const btCollisionObject* collisionObject)
{
	if (!collisionObject)
		return ~0U;

	BodyBullet* body = static_cast< BodyBullet* >(collisionObject->getUserPointer());
	if (!body)
		return ~0U;

	return body->getClusterId();
}

struct ClosestConvexExcludeResultCallback : public btCollisionWorld::ClosestConvexResultCallback
{
	const BodyBullet* m_ignoreBody;
	const QueryFilter& m_queryFilter;
	uint32_t m_group;
	uint32_t m_ignoreClusterId;

	ClosestConvexExcludeResultCallback(const BodyBullet* ignoreBody, const QueryFilter& queryFilter, const btVector3& convexFromWorld, const btVector3& convexToWorld)
	:	btCollisionWorld::ClosestConvexResultCallback(convexFromWorld, convexToWorld)
	,	m_ignoreBody(ignoreBody)
	,	m_queryFilter(queryFilter)
	{
	}

	virtual	btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace) override final
	{
		T_ASSERT(convexResult.m_hitFraction <= m_closestHitFraction);

		BodyBullet* body = static_cast< BodyBullet* >(convexResult.m_hitCollisionObject->getUserPointer());
		if (m_ignoreBody == body)
			return 1.0f;

		if (m_queryFilter.ignoreClusterId != 0 && getClusterId(convexResult.m_hitCollisionObject) == m_queryFilter.ignoreClusterId)
			return 1.0f;

		const uint32_t group = getCollisionGroup(convexResult.m_hitCollisionObject);

		if ((group & m_queryFilter.includeGroup) == 0 || (group & m_queryFilter.ignoreGroup) != 0)
			return 1.0f;

#if 0
		// Fix up hit normal, Bullet always report "edge" normal which is incorrect sometimes.
		if (convexResult.m_localShapeInfo)
		{
			const btCollisionShape* collisionShape = convexResult.m_hitCollisionObject->getCollisionShape();
			if (collisionShape->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE)
			{
				const btTriangleMeshShape* meshShape = reinterpret_cast< const btTriangleMeshShape* >(collisionShape);
				const MeshProxyIndexVertexArray* meshInterface = reinterpret_cast< const MeshProxyIndexVertexArray* >(meshShape->getMeshInterface());

				Vector4 triangleNormal;
				if (meshInterface->getTriangleNormal(convexResult.m_localShapeInfo->m_triangleIndex, triangleNormal))
				{
					Vector4 worldNormal = body->getTransform() * triangleNormal.xyz0();
					m_hitNormalWorld = toBtVector3(worldNormal);
				}
			}
		}
#endif

		return btCollisionWorld::ClosestConvexResultCallback::addSingleResult(convexResult, normalInWorldSpace);
	}
};

struct ClosestRayExcludeResultCallback : public btCollisionWorld::RayResultCallback
{
	btVector3 m_rayFromWorld;
	btVector3 m_rayToWorld;
	btVector3 m_hitNormalWorld;
	btVector3 m_hitPointWorld;
	const QueryFilter& m_queryFilter;
	uint32_t m_queryTypes;
	int32_t m_triangleIndex;

	ClosestRayExcludeResultCallback(const QueryFilter& queryFilter, uint32_t queryTypes, const btVector3& rayFromWorld, const btVector3& rayToWorld)
	:	m_rayFromWorld(rayFromWorld)
	,	m_rayToWorld(rayToWorld)
	,	m_queryFilter(queryFilter)
	,	m_queryTypes(queryTypes)
	,	m_triangleIndex(-1)
	{
		m_flags |= btTriangleRaycastCallback::kF_DisableHeightfieldAccelerator;
	}

	virtual bool needsCollision(btBroadphaseProxy* proxy0) const override final
	{
		return true;
	}

	virtual	btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace) override final
	{
		T_ASSERT(rayResult.m_hitFraction <= m_closestHitFraction);

		if (m_queryFilter.ignoreClusterId != 0 && getClusterId(rayResult.m_collisionObject) == m_queryFilter.ignoreClusterId)
			return m_closestHitFraction;

		const uint32_t group = getCollisionGroup(rayResult.m_collisionObject);

		if ((group & m_queryFilter.includeGroup) == 0 || (group & m_queryFilter.ignoreGroup) != 0)
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

			if (rayResult.m_localShapeInfo)
				m_triangleIndex = rayResult.m_localShapeInfo->m_triangleIndex;
			else
				m_triangleIndex = -1;
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
	const QueryFilter& m_queryFilter;
	int32_t m_triangleIndex;

	ClosestRayExcludeAndCullResultCallback(const QueryFilter& queryFilter, const btVector3& rayFromWorld, const btVector3& rayToWorld)
	:	m_rayFromWorld(rayFromWorld)
	,	m_rayToWorld(rayToWorld)
	,	m_queryFilter(queryFilter)
	,	m_triangleIndex(-1)
	{
		m_flags |= btTriangleRaycastCallback::kF_DisableHeightfieldAccelerator;
	}

	virtual bool needsCollision(btBroadphaseProxy* proxy0) const override final
	{
		return true;
	}

	virtual	btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace) override final
	{
		T_ASSERT(rayResult.m_hitFraction <= m_closestHitFraction);

		if (m_queryFilter.ignoreClusterId != 0 && getClusterId(rayResult.m_collisionObject) == m_queryFilter.ignoreClusterId)
			return m_closestHitFraction;

		const uint32_t group = getCollisionGroup(rayResult.m_collisionObject);

		if ((group & m_queryFilter.includeGroup) == 0 || (group & m_queryFilter.ignoreGroup) != 0)
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

		if (rayResult.m_localShapeInfo)
			m_triangleIndex = rayResult.m_localShapeInfo->m_triangleIndex;
		else
			m_triangleIndex = -1;

		return m_closestHitFraction;
	}
};

struct ConvexExcludeResultCallback : public btCollisionWorld::ConvexResultCallback
{
	const QueryFilter& m_queryFilter;
	AlignedVector< QueryResult >& m_outResult;

	ConvexExcludeResultCallback(const QueryFilter& queryFilter, AlignedVector< QueryResult >& outResult)
	:	m_queryFilter(queryFilter)
	,	m_outResult(outResult)
	{
	}

	virtual bool needsCollision(btBroadphaseProxy* proxy0) const override final
	{
		return true;
	}

	virtual	btScalar addSingleResult(btCollisionWorld::LocalConvexResult& convexResult, bool normalInWorldSpace) override final
	{
		BodyBullet* bodyBullet = reinterpret_cast< BodyBullet* >(convexResult.m_hitCollisionObject->getUserPointer());
		T_ASSERT(bodyBullet);

		if (m_queryFilter.ignoreClusterId != 0 && bodyBullet->getClusterId() == m_queryFilter.ignoreClusterId)
			return 1.0f;

		const uint32_t group = bodyBullet->getCollisionGroup();

		if ((group & m_queryFilter.includeGroup) == 0 || (group & m_queryFilter.ignoreGroup) != 0)
			return 1.0f;

		QueryResult result;
		result.body = bodyBullet;
		result.position = fromBtVector3(convexResult.m_hitPointLocal, 1.0f);

		if (normalInWorldSpace)
			result.normal = fromBtVector3(convexResult.m_hitNormalLocal, 0.0f);
		else
			result.normal = fromBtVector3(convexResult.m_hitCollisionObject->getWorldTransform().getBasis() * convexResult.m_hitNormalLocal, 0.0f);

		result.fraction = convexResult.m_hitFraction;
		result.material = 0;

		m_outResult.push_back(result);
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

	virtual bool needsCollision(btBroadphaseProxy* proxy0) const override final
	{
		return true;
	}

#if 0
	virtual	btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0, int partId0, int index0, const btCollisionObjectWrapper* colObj1, int partId1, int index1) override final
	{
		if (m_colObj == colObj0->getCollisionObject())
		{
			T_ASSERT(colObj1);

			BodyBullet* bodyBullet = reinterpret_cast< BodyBullet* >(colObj1->getCollisionObject()->getUserPointer());
			T_ASSERT(bodyBullet);

			m_outResult.push_back(bodyBullet);
		}
		else if (m_colObj == colObj1->getCollisionObject())
		{
			T_ASSERT(colObj0);

			BodyBullet* bodyBullet = reinterpret_cast< BodyBullet* >(colObj0->getCollisionObject()->getUserPointer());
			T_ASSERT(bodyBullet);

			m_outResult.push_back(bodyBullet);
		}
		return 0.0f;
	}
#else
	virtual	btScalar addSingleResult(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) override final
	{
		const btCollisionObject* colObj0 = colObj0Wrap->getCollisionObject();
		const btCollisionObject* colObj1 = colObj1Wrap->getCollisionObject();

		if (m_colObj == colObj0)
		{
			T_ASSERT(colObj1);

			BodyBullet* bodyBullet = reinterpret_cast< BodyBullet* >(colObj1->getUserPointer());
			T_ASSERT(bodyBullet);

			m_outResult.push_back(bodyBullet);
		}
		else if (m_colObj == colObj1)
		{
			T_ASSERT(colObj0);

			BodyBullet* bodyBullet = reinterpret_cast< BodyBullet* >(colObj0->getUserPointer());
			T_ASSERT(bodyBullet);

			m_outResult.push_back(bodyBullet);
		}

		return 0.0f;
	}
#endif
};

struct QuerySphereCallback : public btBroadphaseAabbCallback
{
	RefArray< BodyBullet > bodies;

	virtual bool process(const btBroadphaseProxy* proxy) override final
	{
		btRigidBody* rigidBody = static_cast< btRigidBody* >(proxy->m_clientObject);
		if (rigidBody)
		{
			BodyBullet* body = reinterpret_cast< BodyBullet* >(rigidBody->getUserPointer());
			T_ASSERT(body);

			bodies.push_back(body);
		}
		return true;
	}
};

class QueryTrianglesCallback : public btTriangleCallback
{
public:
	QueryTrianglesCallback(const btTransform& colT, AlignedVector< TriangleResult >& outTriangles)
	:	m_colT(colT)
	,	m_outTriangles(outTriangles)
	{
	}

	virtual void processTriangle(btVector3* triangle, int partId, int triangleIndex) override final
	{
		TriangleResult tr;
		tr.v[0] = fromBtVector3(m_colT * triangle[0], 1.0f);
		tr.v[1] = fromBtVector3(m_colT * triangle[1], 1.0f);
		tr.v[2] = fromBtVector3(m_colT * triangle[2], 1.0f);
		m_outTriangles.push_back(tr);
	}

private:
	const btTransform& m_colT;
	AlignedVector< TriangleResult >& m_outTriangles;
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
		const int numChildShapes = compoundShape->getNumChildShapes();
		for (int i = 0; i < numChildShapes; ++i)
			deleteShape(compoundShape->getChildShape(i));
	}
	delete shape;
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.PhysicsManagerBullet", 0, PhysicsManagerBullet, PhysicsManager)

PhysicsManagerBullet* PhysicsManagerBullet::ms_this = nullptr;

PhysicsManagerBullet::PhysicsManagerBullet()
:	m_configuration(nullptr)
,	m_dispatcher(nullptr)
,	m_broadphase(nullptr)
,	m_solver(nullptr)
,	m_dynamicsWorld(nullptr)
,	m_queryCountLast(0)
,	m_queryCount(0)
{
}

PhysicsManagerBullet::~PhysicsManagerBullet()
{
	T_ASSERT(!m_configuration);
	T_ASSERT(!m_dispatcher);
	T_ASSERT(!m_broadphase);
	T_ASSERT(!m_solver);
	T_ASSERT(!m_dynamicsWorld);
}

bool PhysicsManagerBullet::create(const PhysicsCreateDesc& desc)
{
	btDefaultCollisionConstructionInfo info;

	// Set our own memory allocator routines instead of Bullet's default.
	btAlignedAllocSetCustom(&traktorAlloc, &traktorFree);
	btAlignedAllocSetCustomAligned(&traktorAllocAlign, &traktorFree);

	m_timeScale = desc.timeScale;
	m_simulationFrequency = desc.simulationFrequency;
	m_configuration = new btDefaultCollisionConfiguration(info);

	m_dispatcher = new btCollisionDispatcher(m_configuration);
	m_solver = new btSequentialImpulseConstraintSolver();
	m_broadphase = new btDbvtBroadphase();
	m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_configuration);
	m_dynamicsWorld->getSolverInfo().m_numIterations = std::max(1, desc.solverIterations);

	m_dispatcher->setNearCallback(&PhysicsManagerBullet::nearCallback);

	// Add our customer contact callback so we can have multiple materials per collision object,
	// since no manager data is accessed this can be set by multiple managers if necessary.
	// But we cannot remove it since it's not reference counted.
	gContactAddedCallback = traktorContactAdded;
	return true;
}

void PhysicsManagerBullet::destroy()
{
	T_ANONYMOUS_VAR(RefArray< Joint >)(m_joints);
	T_ANONYMOUS_VAR(RefArray< BodyBullet >)(m_bodies);

	while (!m_joints.empty())
		m_joints.front()->destroy();
	while (!m_bodies.empty())
		m_bodies.front()->destroy();

	m_joints.clear();
	m_bodies.clear();

	delete m_dynamicsWorld; m_dynamicsWorld = nullptr;
	delete m_solver; m_solver = nullptr;
	delete m_broadphase; m_broadphase = nullptr;
	delete m_dispatcher; m_dispatcher = nullptr;
	delete m_configuration; m_configuration = nullptr;
}

void PhysicsManagerBullet::setGravity(const Vector4& gravity)
{
	T_ASSERT(m_dynamicsWorld);
	m_dynamicsWorld->setGravity(toBtVector3(gravity));
}

Vector4 PhysicsManagerBullet::getGravity() const
{
	T_ASSERT(m_dynamicsWorld);
	return fromBtVector3(m_dynamicsWorld->getGravity(), 0.0f);
}

Ref< Body > PhysicsManagerBullet::createBody(resource::IResourceManager* resourceManager, const BodyDesc* desc, const wchar_t* const tag)
{
	if (!desc)
		return nullptr;

	const ShapeDesc* shapeDesc = desc->getShape();
	if (!shapeDesc)
	{
		log::error << L"Unable to create body, no shape defined." << Endl;
		return nullptr;
	}

	// Create collision shape.
	Vector4 centerOfGravity = Vector4::origo();
	btCollisionShape* shape = nullptr;
	resource::Proxy< Mesh > mesh;

	if (const BoxShapeDesc* boxShape = dynamic_type_cast< const BoxShapeDesc* >(shapeDesc))
	{
		shape = new btBoxShape(toBtVector3(boxShape->getExtent()));
		shape->setMargin(boxShape->getMargin());
	}
	else if (const CapsuleShapeDesc* capsuleShape = dynamic_type_cast< const CapsuleShapeDesc* >(shapeDesc))
	{
		const float radius = capsuleShape->getRadius();
		const float length = capsuleShape->getLength() - radius * 2.0f;
		T_ASSERT(length >= 0.0f);
		shape = new btCapsuleShapeZ(radius, length);
	}
	else if (const CylinderShapeDesc* cylinderShape = dynamic_type_cast< const CylinderShapeDesc* >(shapeDesc))
	{
		const float radius = cylinderShape->getRadius();
		const float length = cylinderShape->getLength();
		shape = new btCylinderShapeZ(btVector3(radius, radius, length / 2.0f));
	}
	else if (const MeshShapeDesc* meshShape = dynamic_type_cast< const MeshShapeDesc* >(shapeDesc))
	{
		if (!resourceManager->bind(meshShape->getMesh(), mesh))
		{
			log::error << L"Unable to load collision mesh resource " << Guid(meshShape->getMesh()).format() << L"." << Endl;
			return nullptr;
		}

		if (is_a< DynamicBodyDesc >(desc))
		{
			const auto& vertices = mesh->getVertices();
			const auto& hullIndices = mesh->getHullIndices();
			if (vertices.empty() || hullIndices.empty())
			{
				log::error << L"Unable to create body, mesh hull empty." << Endl;
				return nullptr;
			}

			// Build point list, only hull points. Add space at end for storeUnaligned always writes four floats.
			AutoArrayPtr< float, AllocFreeAlign > hullPoints((float*)Alloc::acquireAlign(3 * sizeof(float) * (hullIndices.size() + 1), 16, T_FILE_LINE));
			for (uint32_t i = 0; i < hullIndices.size(); ++i)
				vertices[hullIndices[i]].storeUnaligned(&hullPoints[i * 3]);

			// Create Bullet shape.
			shape = new btConvexHullShape(
				static_cast< const btScalar* >(hullPoints.c_ptr()),
				(int)hullIndices.size(),
				3 * sizeof(float)
			);
		}
		else
		{
			MeshProxyIndexVertexArray* indexVertexArray = new MeshProxyIndexVertexArray(mesh);
			shape = new btBvhTriangleMeshShape(indexVertexArray, true);
		}

		shape->setMargin(mesh->getMargin());

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
			log::error << L"Unable to load heightfield resource." << Endl;
			return nullptr;
		}

		shape = new HeightfieldShapeBullet(heightfield);
	}
	else
	{
		log::error << L"Unsupported shape type \"" << type_name(shapeDesc) << L"\"." << Endl;
		return nullptr;
	}

	T_ASSERT(shape);

	return createBody(resourceManager, shapeDesc, desc, shape, tag, centerOfGravity, mesh);
}

Ref< Body > PhysicsManagerBullet::createBody(resource::IResourceManager* resourceManager, const BodyDesc* desc, const Mesh* mesh, const wchar_t* const tag)
{
	if (!desc)
		return nullptr;

	const ShapeDesc* shapeDesc = desc->getShape();
	if (!shapeDesc)
	{
		log::error << L"Unable to create body, no shape defined." << Endl;
		return nullptr;
	}

	// Create collision shape.
	Vector4 centerOfGravity = Vector4::origo();
	btCollisionShape* shape = nullptr;

	if (is_a< DynamicBodyDesc >(desc))
	{
		const auto& vertices = mesh->getVertices();
		const auto& hullIndices = mesh->getHullIndices();
		if (vertices.empty() || hullIndices.empty())
		{
			log::error << L"Unable to create body, mesh hull empty." << Endl;
			return nullptr;
		}

		// Build point list, only hull points. Add space at end for storeUnaligned always writes four floats.
		AutoArrayPtr< float, AllocFreeAlign > hullPoints((float*)Alloc::acquireAlign(3 * sizeof(float) * (hullIndices.size() + 1), 16, T_FILE_LINE));
		for (uint32_t i = 0; i < hullIndices.size(); ++i)
			vertices[hullIndices[i]].storeUnaligned(&hullPoints[i * 3]);

		// Create Bullet shape.
		shape = new btConvexHullShape(
			static_cast< const btScalar* >(hullPoints.c_ptr()),
			(int)hullIndices.size(),
			3 * sizeof(float)
		);
	}
	else
	{
		MeshProxyIndexVertexArray* indexVertexArray = new MeshProxyIndexVertexArray(resource::Proxy< Mesh >(const_cast< Mesh* >(mesh)));
		shape = new btBvhTriangleMeshShape(indexVertexArray, true);
	}

	shape->setMargin(mesh->getMargin());

	return createBody(resourceManager, shapeDesc, desc, shape, tag, centerOfGravity, resource::Proxy< Mesh >(const_cast< Mesh* >(mesh)));
}

Ref< Joint > PhysicsManagerBullet::createJoint(const JointDesc* desc, const Transform& transform, Body* body1, Body* body2)
{
	if (!desc)
		return nullptr;

	BodyBullet* bb1 = checked_type_cast< BodyBullet* >(body1);
	BodyBullet* bb2 = checked_type_cast< BodyBullet* >(body2);

	btRigidBody* b1 = body1 ? bb1->getBtRigidBody() : nullptr;
	btRigidBody* b2 = body2 ? bb2->getBtRigidBody() : nullptr;

	Ref< Joint > joint;

	if (auto axisDesc = dynamic_type_cast< const AxisJointDesc* >(desc))
	{
		btHingeConstraint* hingeConstraint = nullptr;

		if (b1 && b2)
		{
			const Vector4 anchor = transform * axisDesc->getAnchor().xyz1();
			const Vector4 axis = transform * axisDesc->getAxis().xyz0().normalized();

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
			const Vector4 anchor = transform * axisDesc->getAnchor().xyz1();
			const Vector4 axis = transform * axisDesc->getAxis().xyz0().normalized();

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
	else if (auto ballDesc = dynamic_type_cast< const BallJointDesc* >(desc))
	{
		btPoint2PointConstraint* pointConstraint = nullptr;

		if (b1 && b2)
		{
			const Vector4 anchor = transform * ballDesc->getAnchor().xyz1();
			const Vector4 anchorIn1 = bb1->getBodyTransform().inverse() * anchor;
			const Vector4 anchorIn2 = bb2->getBodyTransform().inverse() * anchor;

			pointConstraint = new btPoint2PointConstraint(
				*b1,
				*b2,
				toBtVector3(anchorIn1),
				toBtVector3(anchorIn2)
			);
		}
		else
		{
			const Vector4 anchor = transform * ballDesc->getAnchor().xyz1();
			const Vector4 anchorIn1 = bb1->getBodyTransform().inverse() * anchor;

			pointConstraint = new btPoint2PointConstraint(
				*b1,
				toBtVector3(anchorIn1)
			);
		}

		joint = new BallJointBullet(this, pointConstraint, bb1, bb2);
	}
	else if (auto coneTwistDesc = dynamic_type_cast< const ConeTwistJointDesc* >(desc))
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
	else if (auto dofDesc = dynamic_type_cast< const DofJointDesc* >(desc))
	{
		btGeneric6DofConstraint* dofConstraint = nullptr;

		if (b1 && b2)
		{
			const Transform Tbody1Inv = body1->getCenterTransform().inverse();
			const Transform Tbody2Inv = body2->getCenterTransform().inverse();

			dofConstraint = new btGeneric6DofConstraint(
				*b1,
				*b2,
				toBtTransform(transform * Tbody1Inv),
				toBtTransform(transform * Tbody2Inv),
				true
			);
		}
		else
		{
			const Transform Tbody1Inv = body1->getCenterTransform().inverse();

			dofConstraint = new btGeneric6DofConstraint(
				*b1,
				toBtTransform(transform * Tbody1Inv),
				false
			);
		}

		dofConstraint->setLimit(0, 0.0f, dofDesc->getTranslate().x ? -1.0f : 0.0f);
		dofConstraint->setLimit(1, 0.0f, dofDesc->getTranslate().y ? -1.0f : 0.0f);
		dofConstraint->setLimit(2, 0.0f, dofDesc->getTranslate().z ? -1.0f : 0.0f);

		dofConstraint->setLimit(3, 0.0f, dofDesc->getRotate().x ? -1.0f : 0.0f);
		dofConstraint->setLimit(4, 0.0f, dofDesc->getRotate().y ? -1.0f : 0.0f);
		dofConstraint->setLimit(5, 0.0f, dofDesc->getRotate().z ? -1.0f : 0.0f);

		joint = new DofJointBullet(this, dofConstraint, bb1, bb2);
	}
	else if (auto hingeDesc = dynamic_type_cast< const HingeJointDesc* >(desc))
	{
		btHingeConstraint* hingeConstraint = nullptr;

		if (b1 && b2)
		{
			const Vector4 anchor = transform * hingeDesc->getAnchor().xyz1();
			const Vector4 axis = transform * hingeDesc->getAxis().xyz0().normalized();

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
			const Vector4 anchor = transform * hingeDesc->getAnchor().xyz1();
			const Vector4 axis = transform * hingeDesc->getAxis().xyz0();

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
				hingeConstraint->setLimit(-FUZZY_EPSILON, FUZZY_EPSILON, 1.0f);
		}

		hingeConstraint->setAngularOnly(hingeDesc->getAngularOnly());

		joint = new HingeJointBullet(this, hingeConstraint, bb1, bb2);
	}
	else if (auto hinge2Desc = dynamic_type_cast< const Hinge2JointDesc* >(desc))
	{
		btHinge2Constraint* hinge2Constraint = nullptr;

		if (b1 && b2)
		{
			const Vector4 anchor = transform * hinge2Desc->getAnchor().xyz1();
			const Vector4 axis1 = transform * hinge2Desc->getAxis1().xyz0().normalized();
			const Vector4 axis2 = transform * hinge2Desc->getAxis2().xyz0().normalized();

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
			return nullptr;

		joint = new Hinge2JointBullet(this, hinge2Constraint, bb1, bb2);
	}

	if (!joint)
	{
		log::error << L"Unable to create joint; unknown joint type \"" << type_name(desc) << L"\"." << Endl;
		return nullptr;
	}

	m_joints.push_back(joint);
	return joint;
}

void PhysicsManagerBullet::update(float simulationDeltaTime, bool issueCollisionEvents)
{
	T_ASSERT(m_dynamicsWorld);

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ANONYMOUS_VAR(Save< PhysicsManagerBullet* >)(ms_this, this);

	// Step simulation.
	const float dT = simulationDeltaTime * m_timeScale;
	m_dynamicsWorld->stepSimulation(dT, 10, 1.0f / m_simulationFrequency);

	// Issue collision events.
	if (issueCollisionEvents)
	{
		CollisionInfo info;

		const int32_t manifoldCount = m_dispatcher->getNumManifolds();
		for (int32_t i = 0; i < manifoldCount; ++i)
		{
			btPersistentManifold* manifold = m_dispatcher->getManifoldByIndexInternal(i);
			T_ASSERT(manifold);

			// Only call to listeners when a new manifold has been created.
			const int32_t contacts = manifold->getNumContacts();
			if (contacts <= 0)
				continue;
			if (manifold->getContactPoint(0).m_userPersistentData != nullptr)
				continue;

			const btRigidBody* body0 = reinterpret_cast< const btRigidBody* >(manifold->getBody0());
			const btRigidBody* body1 = reinterpret_cast< const btRigidBody* >(manifold->getBody1());

			BodyBullet* wrapperBody0 = body0 ? static_cast< BodyBullet* >(body0->getUserPointer()) : nullptr;
			BodyBullet* wrapperBody1 = body1 ? static_cast< BodyBullet* >(body1->getUserPointer()) : nullptr;

			info.body1 = wrapperBody0;
			info.body2 = wrapperBody1;
			info.contacts.resize(0);
			info.contacts.reserve(contacts);

			const int32_t material1 = wrapperBody0->getMaterial();
			const int32_t material2 = wrapperBody1->getMaterial();

			for (int32_t j = 0; j < contacts; ++j)
			{
				const btManifoldPoint& pt = manifold->getContactPoint(j);
				if (pt.getDistance() < 0.0f)
				{
					CollisionContact cc;
					cc.depth = -pt.getDistance();
					cc.normal = fromBtVector3(pt.m_normalWorldOnB, 0.0f);
					cc.position = fromBtVector3(pt.m_positionWorldOnA, 1.0f);
					cc.material1 = material1;
					cc.material2 = material2;
					info.contacts.push_back(cc);
				}
			}

			if (!info.contacts.empty())
			{
				notifyCollisionListeners(info);
				manifold->getContactPoint(0).m_userPersistentData = (void*)1;

				// Only issue one new collision per update; distribute over
				// several updates to prevent CPU overload.
				break;
			}
		}
	}
	else
	{
		const int32_t manifoldCount = m_dispatcher->getNumManifolds();
		for (int32_t i = 0; i < manifoldCount; ++i)
		{
			btPersistentManifold* manifold = m_dispatcher->getManifoldByIndexInternal(i);
			T_ASSERT(manifold);

			manifold->getContactPoint(0).m_userPersistentData = nullptr;
		}
	}

	m_queryCountLast = m_queryCount;
	m_queryCount = 0;
}

void PhysicsManagerBullet::solveConstraints(const RefArray< Body >& bodies, const RefArray< Joint >& joints)
{
	btCollisionObject* btBodies[32];
	btTypedConstraint* btConstraints[32];

	for (uint32_t i = 0; i < bodies.size(); ++i)
		btBodies[i] = static_cast< BodyBullet* >(bodies[i])->getBtRigidBody();

	for (uint32_t i = 0; i < joints.size(); ++i)
		btConstraints[i] = static_cast< btTypedConstraint* >(joints[i]->getInternal());

	btConstraintSolver* constraintSolver = m_dynamicsWorld->getConstraintSolver();
	T_ASSERT(constraintSolver);

	constraintSolver->solveGroup(
		btBodies,
		bodies.size(),
		nullptr,
		0,
		btConstraints,
		(int)joints.size(),
		m_dynamicsWorld->getSolverInfo(),
		nullptr,
		m_dynamicsWorld->getDispatcher()
	);
}

RefArray< Body > PhysicsManagerBullet::getBodies() const
{
	return (RefArray< Body >&)m_bodies;
}

uint32_t PhysicsManagerBullet::getCollidingPairs(std::vector< CollisionPair >& outCollidingPairs) const
{
	const int manifoldCount = m_dispatcher->getNumManifolds();

	outCollidingPairs.reserve(manifoldCount);
	for (int i = 0; i < manifoldCount; ++i)
	{
		const btPersistentManifold* manifold = m_dispatcher->getManifoldByIndexInternal(i);
		T_ASSERT(manifold);

		bool validContact = false;
		const int contacts = manifold->getNumContacts();
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

		Body* wrapperBody0 = body0 ? static_cast< Body* >(body0->getUserPointer()) : nullptr;
		Body* wrapperBody1 = body1 ? static_cast< Body* >(body1->getUserPointer()) : nullptr;

		CollisionPair pair = { wrapperBody0, wrapperBody1 };
		outCollidingPairs.push_back(pair);
	}

	return manifoldCount;
}

bool PhysicsManagerBullet::queryPoint(const Vector4& at, float margin, QueryResult& outResult) const
{
	++m_queryCount;
	return false;
}

bool PhysicsManagerBullet::queryRay(
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	const QueryFilter& queryFilter,
	bool ignoreBackFace,
	QueryResult& outResult
) const
{
	++m_queryCount;

	btVector3 from = toBtVector3(at);
	btVector3 to = toBtVector3(at + direction * Scalar(maxLength));

	if (!ignoreBackFace)
	{
		ClosestRayExcludeResultCallback callback(queryFilter, QtAll, from, to);
		m_dynamicsWorld->rayTest(from, to, callback);
		if (!callback.hasHit())
			return false;

		BodyBullet* body = reinterpret_cast< BodyBullet* >(callback.m_collisionObject->getUserPointer());
		T_ASSERT(body);

		outResult.body = body;
		outResult.position = fromBtVector3(callback.m_hitPointWorld, 1.0f);
		outResult.normal = fromBtVector3(callback.m_hitNormalWorld, 0.0).normalized();
		outResult.distance = dot3(direction, outResult.position - at);
		outResult.material = body->getMaterial();

		if (callback.m_triangleIndex >= 0)
		{
			const btCollisionShape* collisionShape = callback.m_collisionObject->getCollisionShape();
			if (collisionShape->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE)
			{
				const btTriangleMeshShape* meshShape = reinterpret_cast< const btTriangleMeshShape* >(collisionShape);
				const MeshProxyIndexVertexArray* meshInterface = reinterpret_cast< const MeshProxyIndexVertexArray* >(meshShape->getMeshInterface());

				Vector4 triangleNormal = outResult.normal;
				meshInterface->getTriangleNormal(callback.m_triangleIndex, triangleNormal);
				outResult.normal = body->getTransform() * triangleNormal.xyz0();
			}
		}
	}
	else
	{
		ClosestRayExcludeAndCullResultCallback callback(queryFilter, from, to);
		m_dynamicsWorld->rayTest(from, to, callback);
		if (!callback.hasHit())
			return false;

		BodyBullet* body = reinterpret_cast< BodyBullet* >(callback.m_collisionObject->getUserPointer());
		T_ASSERT(body);

		outResult.body = body;
		outResult.position = fromBtVector3(callback.m_hitPointWorld, 1.0f);
		outResult.normal = fromBtVector3(callback.m_hitNormalWorld, 0.0).normalized();
		outResult.distance = dot3(direction, outResult.position - at);
		outResult.material = body->getMaterial();

		if (callback.m_triangleIndex >= 0)
		{
			const btCollisionShape* collisionShape = callback.m_collisionObject->getCollisionShape();
			if (collisionShape->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE)
			{
				const btTriangleMeshShape* meshShape = reinterpret_cast< const btTriangleMeshShape* >(collisionShape);
				const MeshProxyIndexVertexArray* meshInterface = reinterpret_cast< const MeshProxyIndexVertexArray* >(meshShape->getMeshInterface());

				Vector4 triangleNormal = outResult.normal;
				meshInterface->getTriangleNormal(callback.m_triangleIndex, triangleNormal);
				outResult.normal = body->getTransform() * triangleNormal.xyz0();
			}
		}
	}

	return true;
}

bool PhysicsManagerBullet::queryShadowRay(
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	const QueryFilter& queryFilter,
	uint32_t queryTypes
) const
{
	++m_queryCount;

	const btVector3 from = toBtVector3(at);
	const btVector3 to = toBtVector3(at + direction * Scalar(maxLength));

	ClosestRayExcludeResultCallback callback(queryFilter, queryTypes, from, to);
	m_dynamicsWorld->rayTest(from, to, callback);
	if (!callback.hasHit())
		return false;

	return true;
}

uint32_t PhysicsManagerBullet::querySphere(
	const Vector4& at,
	float radius,
	const QueryFilter& queryFilter,
	uint32_t queryTypes,
	RefArray< Body >& outBodies
) const
{
	++m_queryCount;

	outBodies.resize(0);

	const btVector3 center = toBtVector3(at);
	const btVector3 radii = btVector3(radius, radius, radius);

	const btVector3 aabbMin = center - radii;
	const btVector3 aabbMax = center + radii;

	QuerySphereCallback callback;
	m_broadphase->aabbTest(aabbMin, aabbMax, callback);
	for (auto body : callback.bodies)
	{
		const uint32_t group = body->getCollisionGroup();

		if ((group & queryFilter.includeGroup) == 0 || (group & queryFilter.ignoreGroup) != 0)
			continue;

		const bool st = body->isStatic();
		if ((queryTypes & QtStatic) == 0 && st)
			continue;
		if ((queryTypes & QtDynamic) == 0 && !st)
			continue;

		btRigidBody* rigidBody = body->getBtRigidBody();
		T_ASSERT(rigidBody);

		btVector3 aabbMin, aabbMax;
		rigidBody->getAabb(aabbMin, aabbMax);

		const float bodyRadius = (aabbMax - aabbMin).length() * 0.5f;
		const Vector4 bodyCenter = fromBtVector3((aabbMin + aabbMax) * 0.5f, 1.0f);

		if ((bodyCenter - at).length() - radius - bodyRadius <= 0.0f)
			outBodies.push_back(body);
	}

	return uint32_t(outBodies.size());
}

bool PhysicsManagerBullet::querySweep(
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	float radius,
	const QueryFilter& queryFilter,
	QueryResult& outResult
) const
{
	++m_queryCount;

	const btSphereShape sphereShape(radius);
	btTransform from, to;

	from.setIdentity();
	from.setOrigin(toBtVector3(at));

	to.setIdentity();
	to.setOrigin(toBtVector3(at + direction * Scalar(maxLength)));

	ClosestConvexExcludeResultCallback callback(0, queryFilter, from.getOrigin(), to.getOrigin());
	m_dynamicsWorld->convexSweepTest(
		&sphereShape,
		from,
		to,
		callback
	);
	if (!callback.hasHit())
		return false;

	BodyBullet* body = reinterpret_cast< BodyBullet* >(callback.m_hitCollisionObject->getUserPointer());
	T_ASSERT(body);

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
	const QueryFilter& queryFilter,
	QueryResult& outResult
) const
{
	++m_queryCount;

	btRigidBody* rigidBody = checked_type_cast< const BodyBullet* >(body)->getBtRigidBody();
	btCollisionShape* shape = rigidBody->getCollisionShape();

	// If shape is a compound we assume it's first child is a convex shape.
	btQuaternion localRotation(0.0f, 0.0f, 0.0f, 1.0f);
	if (shape->isCompound())
	{
		btCompoundShape* compoundShape = static_cast< btCompoundShape* >(shape);
		if (compoundShape->getNumChildShapes() > 0)
		{
			localRotation = compoundShape->getChildTransform(0).getRotation();
			shape = compoundShape->getChildShape(0);
		}
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

	ClosestConvexExcludeResultCallback callback(checked_type_cast< const BodyBullet* >(body), queryFilter, from.getOrigin(), to.getOrigin());
	m_dynamicsWorld->convexSweepTest(
		static_cast< const btConvexShape* >(shape),
		from,
		to,
		callback
	);
	if (!callback.hasHit())
		return false;

	BodyBullet* bodyBullet = reinterpret_cast< BodyBullet* >(callback.m_hitCollisionObject->getUserPointer());
	T_ASSERT(bodyBullet);

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
	const QueryFilter& queryFilter,
	AlignedVector< QueryResult >& outResult
) const
{
	++m_queryCount;

	const btSphereShape sphereShape(radius);
	btTransform from, to;

	from.setIdentity();
	from.setOrigin(toBtVector3(at));

	to.setIdentity();
	to.setOrigin(toBtVector3(at + direction * Scalar(maxLength)));

	ConvexExcludeResultCallback callback(
		queryFilter,
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
	//++m_queryCount;

	//btRigidBody* rigidBody = checked_type_cast< const BodyBullet* >(body)->getBtRigidBody();
	//T_ASSERT(rigidBody);

	//ContactResultCallback callback(rigidBody, outResult);
	//m_dynamicsWorld->contactTest(rigidBody, callback);
}

void PhysicsManagerBullet::queryTriangles(const Vector4& center, float radius, AlignedVector< TriangleResult >& outTriangles) const
{
	const btCollisionObjectArray& collisionObjects = m_dynamicsWorld->getCollisionObjectArray();
	for (int i = 0; i < collisionObjects.size(); ++i)
	{
		btCollisionObject* col = collisionObjects[i];
		T_ASSERT(col);

		btCollisionShape* shape = col->getCollisionShape();
		if (!shape)
			continue;

		const btTransform& colT = col->getWorldTransform();
		const btTransform colTinv = colT.inverse();

		QueryTrianglesCallback trianglesCallback(colT, outTriangles);

		if (shape->isCompound())
		{
			btCompoundShape* compound = static_cast< btCompoundShape* >(shape);

			const int numChilds = compound->getNumChildShapes();
			for (int j = 0; j < numChilds; ++j)
			{
				btCollisionShape* childShape = compound->getChildShape(j);
				T_ASSERT(childShape);

				if (childShape->isConcave())
				{
					const btVector3 localCenter = colTinv * toBtVector3(center);
					const btVector3 aabbMin = localCenter - btVector3(radius, radius, radius);
					const btVector3 aabbMax = localCenter + btVector3(radius, radius, radius);

					btConcaveShape* concave = static_cast< btConcaveShape* >(childShape);
					concave->processAllTriangles(&trianglesCallback, aabbMin, aabbMax);
				}
			}
		}
		else if (shape->isConcave())
		{
			const btVector3 localCenter = colTinv * toBtVector3(center);
			const btVector3 aabbMin = localCenter - btVector3(radius, radius, radius);
			const btVector3 aabbMax = localCenter + btVector3(radius, radius, radius);

			btConcaveShape* concave = static_cast< btConcaveShape* >(shape);
			concave->processAllTriangles(&trianglesCallback, aabbMin, aabbMax);
		}
	}
}

void PhysicsManagerBullet::getStatistics(PhysicsStatistics& outStatistics) const
{
	outStatistics.bodyCount = 0;
	outStatistics.activeCount = 0;
	outStatistics.manifoldCount = 0;
	outStatistics.queryCount = m_queryCountLast;

	const btCollisionObjectArray& collisionObjects = m_dynamicsWorld->getCollisionObjectArray();
	for (int i = 0; i < collisionObjects.size(); ++i)
	{
		++outStatistics.bodyCount;
		if (collisionObjects[i]->isActive())
			++outStatistics.activeCount;
	}

	outStatistics.manifoldCount = m_dispatcher->getNumManifolds();
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

	// Destroy joints which still reference body begin destroyed, this is a bad pattern
	// since it means we're destroying bodies before joints but since we cannot
	// assume order from components we need to handle it.
	RefArray< Joint > joints = m_joints;
	for (auto joint : joints)
	{
		if (joint->getBody1() == body || joint->getBody2() == body)
			joint->destroy();
	}

	T_FATAL_ASSERT(rigidBody->getNumConstraintRefs() == 0);
	m_dynamicsWorld->removeRigidBody(rigidBody);

	const bool removed = m_bodies.remove(body);
	T_FATAL_ASSERT(removed);

	delete rigidBody->getMotionState();
	delete rigidBody;
	deleteShape(shape);
}

void PhysicsManagerBullet::destroyConstraint(Joint* joint, btTypedConstraint* constraint)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	m_dynamicsWorld->removeConstraint(constraint);

	const bool removed = m_joints.remove(joint);
	T_FATAL_ASSERT(removed);

	delete constraint;
}

Ref< Body > PhysicsManagerBullet::createBody(resource::IResourceManager* resourceManager, const ShapeDesc* shapeDesc, const BodyDesc* desc, btCollisionShape* shape, const wchar_t* const tag, Vector4 centerOfGravity, const resource::Proxy< Mesh >& mesh)
{
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
		info.m_restitution = staticDesc->getRestitution();

		btRigidBody* rigidBody = new btRigidBody(info);

		// Set static flag.
		rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);

		// Define body as kinematic.
		if (staticDesc->isKinematic())
		{
			rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
			rigidBody->setActivationState(DISABLE_DEACTIVATION);
		}

		// Add custom material callback so we can support material per triangle.
		if (shape->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE)
			rigidBody->setCollisionFlags(rigidBody->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);

		// Resolve collision group and mask value.
		uint32_t mergedCollisionGroup = 0;
		for (const auto& group : shapeDesc->getCollisionGroup())
		{
			resource::Proxy< CollisionSpecification > collisionGroup;
			if (!resourceManager->bind(group, collisionGroup))
			{
				log::error << L"Unable to bind collision group specification." << Endl;
				return nullptr;
			}
			mergedCollisionGroup |= collisionGroup->getBitMask();
		}

		uint32_t mergedCollisionMask = 0;
		for (const auto& mask : shapeDesc->getCollisionMask())
		{
			resource::Proxy< CollisionSpecification > collisionMask;
			if (!resourceManager->bind(mask, collisionMask))
			{
				log::error << L"Unable to bind collision mask specification." << Endl;
				return nullptr;
			}
			mergedCollisionMask |= collisionMask->getBitMask();
		}

		// Create our wrapper.
		Ref< BodyBullet > staticBody = new BodyBullet(
			tag,
			this,
			m_dynamicsWorld,
			m_timeScale,
			rigidBody,
			shape,
			centerOfGravity,
			mergedCollisionGroup,
			mergedCollisionMask,
			shapeDesc->getMaterial(),
			mesh
		);
		m_bodies.push_back(staticBody);

		rigidBody->setUserPointer(staticBody);
		body = staticBody;
	}
	else if (const DynamicBodyDesc* dynamicDesc = dynamic_type_cast<const DynamicBodyDesc*>(desc))
	{
		const float mass = dynamicDesc->getMass();

		// Calculate inertia from shape.
		btVector3 localInertia(0.0f, 0.0f, 0.0f);
		shape->calculateLocalInertia(mass, localInertia);

		// Create body.
		btRigidBody::btRigidBodyConstructionInfo info(mass, new btDefaultMotionState(), shape, localInertia);
		info.m_linearDamping = dynamicDesc->getLinearDamping();
		info.m_angularDamping = dynamicDesc->getAngularDamping();
		info.m_friction = dynamicDesc->getFriction();
		info.m_restitution = dynamicDesc->getRestitution();
		info.m_linearSleepingThreshold = dynamicDesc->getLinearThreshold();
		info.m_angularSleepingThreshold = dynamicDesc->getAngularThreshold();
		btRigidBody* rigidBody = new btRigidBody(info);

		if (!dynamicDesc->getActive())
		{
			T_ASSERT_M(dynamicDesc->getAutoDeactivate(), L"If body is initially disabled then auto deactivate must be set as well");
			rigidBody->forceActivationState(ISLAND_SLEEPING);
		}
		else
		{
			if (!dynamicDesc->getAutoDeactivate())
				rigidBody->forceActivationState(DISABLE_DEACTIVATION);
			else
				rigidBody->forceActivationState(ACTIVE_TAG);
		}

		// Resolve collision group and mask value.
		uint32_t mergedCollisionGroup = 0;
		for (const auto& group : shapeDesc->getCollisionGroup())
		{
			resource::Proxy< CollisionSpecification > collisionGroup;
			if (!resourceManager->bind(group, collisionGroup))
			{
				log::error << L"Unable to bind collision group specification." << Endl;
				return nullptr;
			}
			mergedCollisionGroup |= collisionGroup->getBitMask();
		}

		uint32_t mergedCollisionMask = 0;
		for (const auto& mask : shapeDesc->getCollisionMask())
		{
			resource::Proxy< CollisionSpecification > collisionMask;
			if (!resourceManager->bind(mask, collisionMask))
			{
				log::error << L"Unable to bind collision mask specification." << Endl;
				return nullptr;
			}
			mergedCollisionMask |= collisionMask->getBitMask();
		}

		// Create our wrapper.
		Ref< BodyBullet > dynamicBody = new BodyBullet(
			tag,
			this,
			m_dynamicsWorld,
			m_timeScale,
			rigidBody,
			shape,
			centerOfGravity,
			mergedCollisionGroup,
			mergedCollisionMask,
			shapeDesc->getMaterial(),
			mesh
		);
		m_bodies.push_back(dynamicBody);

		rigidBody->setUserPointer(dynamicBody);
		body = dynamicBody;
	}
	else
	{
		log::error << L"Unsupported body type \"" << type_name(desc) << L"\"." << Endl;
		return nullptr;
	}

	// Ensure wrapper body is placed at origo in order
	// to set COG offset in Bullet body.
	if (body)
		body->setTransform(Transform::identity());

	return body;
}

void PhysicsManagerBullet::nearCallback(btBroadphasePair& collisionPair, btCollisionDispatcher& dispatcher, const btDispatcherInfo& dispatchInfo)
{
	T_ASSERT(ms_this);

	const btCollisionObject* colObj0 = static_cast< const btCollisionObject* >(collisionPair.m_pProxy0->m_clientObject);
	const btCollisionObject* colObj1 = static_cast< const btCollisionObject* >(collisionPair.m_pProxy1->m_clientObject);

	const BodyBullet* body1 = colObj0 ? static_cast< const BodyBullet* >(colObj0->getUserPointer()) : nullptr;
	const BodyBullet* body2 = colObj1 ? static_cast< const BodyBullet* >(colObj1->getUserPointer()) : nullptr;
	if (body1 && body2)
	{
		// Filter on cluster id.
		const uint32_t clusterId1 = body1->getClusterId();
		if (clusterId1 != ~0U && clusterId1 == body2->getClusterId())
			return;

		// Filter collision on collision group and mask.
		const uint32_t group1 = body1->getCollisionGroup();
		const uint32_t mask1 = body1->getCollisionMask();

		const uint32_t group2 = body2->getCollisionGroup();
		const uint32_t mask2 = body2->getCollisionMask();

		if ((group1 & mask2) == 0 || (group2 & mask1) == 0)
			return;
	}

	btCollisionDispatcher::defaultNearCallback(collisionPair, dispatcher, dispatchInfo);
}

}
