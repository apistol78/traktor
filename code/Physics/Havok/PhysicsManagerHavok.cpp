#include <algorithm>
/*lint -e49*/
#include <Common/Base/hkBase.h>
#include <Common/Base/Memory/Memory/Pool/hkPoolMemory.h>
#include <Common/Base/Types/Geometry/hkGeometry.h>
#include <Common/Base/Types/Geometry/hkStridedVertices.h>
#include <Physics/Dynamics/World/hkpWorld.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include <Physics/Dynamics/Constraint/Bilateral/BallAndSocket/hkpBallAndSocketConstraintData.h>
#include <Physics/Dynamics/Constraint/Bilateral/Hinge/hkpHingeConstraintData.h>
#include <Physics/Utilities/Dynamics/Inertia/hkpInertiaTensorComputer.h>
#include <Physics/Collide/Dispatch/hkpAgentRegisterUtil.h>
#include <Physics/Collide/Agent/Collidable/hkpCdPoint.h>
#include <Physics/Collide/Shape/Convex/Sphere/hkpSphereShape.h>
#include <Physics/Collide/Shape/Convex/Box/hkpBoxShape.h>
#include <Physics/Collide/Shape/Convex/Cylinder/hkpCylinderShape.h>
#include <Physics/Collide/Shape/Convex/ConvexVertices/hkpConvexVerticesShape.h>
#include <Physics/Collide/Shape/Convex/ConvexVertices/hkpConvexVerticesConnectivityUtil.h>
#include <Physics/Collide/Shape/Compound/Collection/SimpleMesh/hkpSimpleMeshShape.h>
#include <Physics/Collide/Shape/Compound/Tree/Mopp/hkpMoppBvTreeShape.h>
#include <Physics/Collide/Shape/Compound/Tree/Mopp/hkpMoppUtility.h>
#include <Physics/Collide/Shape/HeightField/SampledHeightField/hkpSampledHeightFieldShape.h>
#include <Physics/Collide/Shape/Misc/Transform/hkpTransformShape.h>
#include <Physics/Collide/Query/CastUtil/hkpWorldRayCastInput.h>
#include <Physics/Collide/Query/CastUtil/hkpLinearCastInput.h>
#include <Physics/Collide/Query/Collector/RayCollector/hkpClosestRayHitCollector.h>
#include <Physics/Collide/Query/Collector/PointCollector/hkpSimpleClosestContactCollector.h>
/*lint -restore*/
#include "Core/Log/Log.h"
#include "Core/Math/Plane.h"
#include "Core/Misc/TString.h"
#include "Heightfield/Heightfield.h"
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
#include "Physics/Havok/BallJointHavok.h"
#include "Physics/Havok/BodyHavok.h"
#include "Physics/Havok/ConeTwistJointHavok.h"
#include "Physics/Havok/Conversion.h"
#include "Physics/Havok/HingeJointHavok.h"
#include "Physics/Havok/PhysicsManagerHavok.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace physics
	{
		namespace
		{

class IgnoreBodyClosestRayHitCollector : public hkpClosestRayHitCollector
{
public:
	IgnoreBodyClosestRayHitCollector(const hkpCollidable* ignoreCollidable)
	:	m_ignoreCollidable(ignoreCollidable)
	{
	}

protected:
	virtual void addRayHit(const hkpCdBody& cdBody, const hkpShapeRayCastCollectorOutput& hitInfo) 
	{
		if (cdBody.getRootCollidable() != m_ignoreCollidable)
			hkpClosestRayHitCollector::addRayHit(cdBody, hitInfo);
	}

private:
	const hkpCollidable* m_ignoreCollidable;
};

class IgnoreBodyClosestContactCollector : public hkpSimpleClosestContactCollector
{
public:
	IgnoreBodyClosestContactCollector(const hkpCollidable* ignoreCollidable)
	:	m_ignoreCollidable(ignoreCollidable)
	{
	}

protected:
	virtual void addCdPoint(const hkpCdPoint& pointInfo)
	{
		if (
			pointInfo.m_cdBodyA.getRootCollidable() != m_ignoreCollidable &&
			pointInfo.m_cdBodyB.getRootCollidable() != m_ignoreCollidable
		)
			hkpSimpleClosestContactCollector::addCdPoint(pointInfo);
	}

private:
	const hkpCollidable* m_ignoreCollidable;
};

class HeightfieldShape : public hkpSampledHeightFieldShape
{
public:
	HeightfieldShape(const hkpSampledHeightFieldBaseCinfo& ci, hf::Heightfield* heightfield)
	:	hkpSampledHeightFieldShape(ci)
	,	m_heightfield(heightfield)
	{
	}

	HK_FORCE_INLINE hkReal getHeightAtImpl(int x, int z) const
	{
		const hf::height_t* heights = m_heightfield->getHeights();

		x = max< int >(min< int >(x, m_heightfield->getSize()), 0);
		z = max< int >(min< int >(z, m_heightfield->getSize()), 0);

		int offset = x + z * m_heightfield->getSize();

		return ((float)heights[offset] / 32767.0f - 1.0f) * m_heightfield->getWorldExtent().y() * 0.5f;
	}

	HK_FORCE_INLINE hkBool getTriangleFlipImpl() const
	{
		return false;
	}

	virtual void collideSpheres(const CollideSpheresInput& input, SphereCollisionOutput* outputArray) const
	{
		hkSampledHeightFieldShape_collideSpheres(*this, input, outputArray);
	}

private:
	Ref< hf::Heightfield > m_heightfield;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.PhysicsManagerHavok", 0, PhysicsManagerHavok, PhysicsManager)

PhysicsManagerHavok::PhysicsManagerHavok()
:	m_simulationDeltaTime(0.0f)
,	m_memoryManager(0)
,	m_threadMemory(0)
,	m_stackBuffer(0)
,	m_world(0)
{
}

PhysicsManagerHavok::~PhysicsManagerHavok()
{
	T_ASSERT (!m_world);
	m_memoryManager = 0;
	m_threadMemory = 0;
	m_stackBuffer = 0;
	m_world = 0;
}

bool PhysicsManagerHavok::create(float simulationDeltaTime)
{
	m_simulationDeltaTime = simulationDeltaTime;

	// Initialize the base system including memory system.
	m_memoryManager = new hkPoolMemory();
	m_threadMemory = new hkThreadMemory(m_memoryManager);
	hkBaseSystem::init(m_memoryManager, m_threadMemory, &PhysicsManagerHavok::showError);

	// We now initialize the stack area to 100k (fast temporary memory to be used by the engine).
	const int stackSize = 0x100000;
	m_stackBuffer = hkAllocate<uint8_t>(stackSize, HK_MEMORY_CLASS_BASE);
	hkThreadMemory::getInstance().setStackArea(m_stackBuffer, stackSize);

	hkpWorldCinfo info;
	info.m_simulationType = hkpWorldCinfo::SIMULATION_TYPE_DISCRETE;
	info.m_gravity.set(0.0f, -9.8f, 0.0f);
	info.m_collisionTolerance = 0.01f; 
	info.m_broadPhaseBorderBehaviour = hkpWorldCinfo::BROADPHASE_BORDER_FIX_ENTITY;
	info.setBroadPhaseWorldSize(1000.0f);
	info.setupSolverInfo(hkpWorldCinfo::SOLVER_TYPE_4ITERS_MEDIUM);

	m_world = new hkpWorld(info);

	// Register agents.
	hkpAgentRegisterUtil::registerAllAgents(m_world->getCollisionDispatcher());

	return true;
}

void PhysicsManagerHavok::destroy()
{
	T_ASSERT (m_world);

	// Destroy all joints and bodies.
	while (!m_joints.empty())
		m_joints.front()->destroy();
	while (!m_bodies.empty())
		m_bodies.front()->destroy();

	// Release world instance.
	m_world->removeReference();
	m_world = 0;

	// Deallocate stack area
	m_threadMemory->setStackArea(0, 0);
	hkDeallocate(m_stackBuffer);

	m_threadMemory->removeReference();
	m_threadMemory = 0;

	m_memoryManager->removeReference();
	m_memoryManager = 0;

	// Quit base system
	hkBaseSystem::quit();
}

void PhysicsManagerHavok::setGravity(const Vector4& gravity)
{
	T_ASSERT (m_world);
	m_world->setGravity(toHkVector4(gravity));
}

Vector4 PhysicsManagerHavok::getGravity() const
{
	T_ASSERT (m_world);
	return fromHkVector4(m_world->getGravity());
}

Ref< Body > PhysicsManagerHavok::createBody(resource::IResourceManager* resourceManager, const BodyDesc* desc)
{
	if (!desc)
		return 0;

	// \fixme Should probably not call this every time.
	hkBaseSystem::initThread(m_threadMemory);

	const ShapeDesc* shapeDesc = desc->getShape();
	if (!shapeDesc)
	{
		log::error << L"Unable to create body, no shape defined" << Endl;
		return 0;
	}

	HvkRef< hkpShape > shape;

	// Create collision shape.
	if (const BoxShapeDesc* boxShape = dynamic_type_cast< const BoxShapeDesc* >(shapeDesc))
	{
		const Scalar c_boxShellRadius(0.1f);
		shape = new hkpBoxShape(
			toHkVector4(boxShape->getExtent() - c_boxShellRadius),
			c_boxShellRadius
		);
	}
	else if (const CapsuleShapeDesc* capsuleShape = dynamic_type_cast< const CapsuleShapeDesc* >(shapeDesc))
	{
		float radius = capsuleShape->getRadius();
		float length = capsuleShape->getLength() - radius * 2.0f;

		Vector4 vertex1(0.0f, 0.0f, -length / 2.0f);
		Vector4 vertex2(0.0f, 0.0f, length / 2.0f);

		shape = new hkpCylinderShape(
			toHkVector4(vertex1),
			toHkVector4(vertex2),
			radius
		);
	}
	else if (const CylinderShapeDesc* cylinderShape = dynamic_type_cast< const CylinderShapeDesc* >(shapeDesc))
	{
		float radius = cylinderShape->getRadius();
		float length = cylinderShape->getLength();
	
		Vector4 vertex1(0.0f, 0.0f, -length / 2.0f);
		Vector4 vertex2(0.0f, 0.0f, length / 2.0f);

		shape = new hkpCylinderShape(
			toHkVector4(vertex1),
			toHkVector4(vertex2),
			radius
		);
	}
	else if (const MeshShapeDesc* meshShape = dynamic_type_cast< const MeshShapeDesc* >(shapeDesc))
	{
		resource::Proxy< Mesh > mesh;
		if (!resourceManager->bind(meshShape->getMesh(), mesh))
		{
			log::error << L"Unable to load mesh resource" << Endl;
			return 0;
		}

		const AlignedVector< Vector4 >& vertices = mesh->getVertices();
		const std::vector< Mesh::Triangle >& shapeTriangles = mesh->getShapeTriangles();
		//const std::vector< Mesh::Triangle >& hullTriangles = mesh->getHullTriangles();

		if (is_a< DynamicBodyDesc >(desc))
		{
			hkStridedVertices stridedVerts;
			{
				stridedVerts.m_numVertices = int(vertices.size());
				stridedVerts.m_striding = sizeof(Vector4);
				stridedVerts.m_vertices = reinterpret_cast< const float* >(&vertices[0]);
			}

			hkArray<hkVector4> planes;

			//for (std::vector< Mesh::Triangle >::const_iterator i = hullTriangles.begin(); i != hullTriangles.end(); ++i)
			//{
			//	Plane pl(vertices[i->indices[0]], vertices[i->indices[1]], vertices[i->indices[2]]);
			//	planes.pushBack(toHkVector4(pl.eq));
			//}

			hkGeometry geom;
			hkpConvexVerticesConnectivity* connectivity;
			hkpConvexVerticesConnectivityUtil::createConvexGeometry(stridedVerts, geom, planes, &connectivity);

			{
				stridedVerts.m_numVertices = geom.m_vertices.getSize();
				stridedVerts.m_striding = sizeof(hkVector4);
				stridedVerts.m_vertices = &(geom.m_vertices[0](0));
			}

			hkpConvexVerticesShape* convexShape = new hkpConvexVerticesShape(stridedVerts, planes);
			convexShape->setConnectivity(connectivity);

			shape = convexShape;
		}
		else
		{
			hkpSimpleMeshShape* meshShape = new hkpSimpleMeshShape();

			for (AlignedVector< Vector4 >::const_iterator i = vertices.begin(); i != vertices.end(); ++i)
				meshShape->m_vertices.pushBack(toHkVector4(*i));

			for (std::vector< Mesh::Triangle >::const_iterator i = shapeTriangles.begin(); i != shapeTriangles.end(); ++i)
			{
				hkpSimpleMeshShape::Triangle triangle;
				triangle.m_a = i->indices[0];
				triangle.m_b = i->indices[1];
				triangle.m_c = i->indices[2];
				meshShape->m_triangles.pushBack(triangle);
			}

			hkpMoppCompilerInput mci;
			hkpMoppCode* code = hkpMoppUtility::buildCode(meshShape, mci);
			hkpMoppBvTreeShape* moppShape = new hkpMoppBvTreeShape(meshShape, code);
			code->removeReference();
			meshShape->removeReference();

			shape = moppShape;
		}
	}
	else if (const SphereShapeDesc* sphereShape = dynamic_type_cast< const SphereShapeDesc* >(shapeDesc))
	{
		shape = new hkpSphereShape(sphereShape->getRadius());
	}
	else if (const HeightfieldShapeDesc* heightfieldShape = dynamic_type_cast< const HeightfieldShapeDesc* >(shapeDesc))
	{
		resource::Proxy< hf::Heightfield > heightfield;
		if (!resourceManager->bind(heightfieldShape->getHeightfield(), heightfield))
		{
			log::error << L"Unable to load heightfield resource" << Endl;
			return 0;
		}

		hkpSampledHeightFieldBaseCinfo heightfieldInfo;
		heightfieldInfo.m_xRes = heightfield->getSize();
		heightfieldInfo.m_zRes = heightfield->getSize();

		shape = new HeightfieldShape(heightfieldInfo, heightfield);

		// Center shape in world.
		hkTransform transform;
		transform.setIdentity();
		transform.getTranslation().set(
			-0.5f * heightfield->getWorldExtent().x(),
			0.0f,
			-0.5f * heightfield->getWorldExtent().z()
		);
		shape = new hkpTransformShape(
			shape,
			transform
		);
	}
	else
	{
		log::error << L"Unsupported shape type \"" << type_name(shapeDesc) << L"\"" << Endl;
		return 0;
	}

	// Create compound shape which has the shape's local transformation.
	if (shapeDesc->getLocalTransform() != Transform::identity())
	{
		// Build Havok local transformation; need to renormalize rotation.
		hkTransform localTransform = toHkTransform(shapeDesc->getLocalTransform());
		
		localTransform.getRotation().renormalize();
		T_ASSERT (localTransform.isOk());

		shape = new hkpTransformShape(
			shape,
			localTransform
		);
	}

	// Create rigid body.
	Ref< Body > body;

	if (const StaticBodyDesc* staticDesc = dynamic_type_cast< const StaticBodyDesc* >(desc))
	{
		hkpRigidBodyCinfo rigidBodyInfo;
		rigidBodyInfo.m_shape = shape;
		rigidBodyInfo.m_friction = staticDesc->getFriction();
		rigidBodyInfo.m_motionType = staticDesc->isKinematic() ? hkpMotion::MOTION_KEYFRAMED : hkpMotion::MOTION_FIXED;
		rigidBodyInfo.m_qualityType = staticDesc->isKinematic() ? HK_COLLIDABLE_QUALITY_KEYFRAMED : HK_COLLIDABLE_QUALITY_FIXED;
		rigidBodyInfo.m_rigidBodyDeactivatorType = hkpRigidBodyDeactivator::DEACTIVATOR_NEVER;

		HvkRef< hkpRigidBody > rigidBody(new hkpRigidBody(rigidBodyInfo));

		// Add body to world.
		m_world->addEntity(rigidBody);

		// Create our wrapper.
		Ref< BodyHavok > staticBody = new BodyHavok(
			this,
			rigidBody,
			m_simulationDeltaTime
		);
		m_bodies.push_back(staticBody);

		body = staticBody;
	}
	else if (const DynamicBodyDesc* dynamicDesc = dynamic_type_cast< const DynamicBodyDesc* >(desc))
	{
		hkpRigidBodyCinfo rigidBodyInfo;
		rigidBodyInfo.m_shape = shape;
		rigidBodyInfo.m_linearDamping = dynamicDesc->getLinearDamping();
		rigidBodyInfo.m_angularDamping = dynamicDesc->getAngularDamping();
		rigidBodyInfo.m_friction = dynamicDesc->getFriction();
		rigidBodyInfo.m_motionType = hkpMotion::MOTION_BOX_INERTIA;
		rigidBodyInfo.m_qualityType = HK_COLLIDABLE_QUALITY_MOVING;
		rigidBodyInfo.m_rigidBodyDeactivatorType = hkpRigidBodyDeactivator::DEACTIVATOR_SPATIAL;

		hkpInertiaTensorComputer::setShapeVolumeMassProperties(
			shape,
			dynamicDesc->getMass(),
			rigidBodyInfo
		);

		HvkRef< hkpRigidBody > rigidBody(new hkpRigidBody(rigidBodyInfo));

		// Add body to world.
		m_world->addEntity(rigidBody);

		// Create our wrapper.
		Ref< BodyHavok > dynamicBody = new BodyHavok(
			this,
			rigidBody,
			m_simulationDeltaTime
		);
		m_bodies.push_back(dynamicBody);

		body = dynamicBody;
	}
	else
	{
		log::error << L"Unsupported body type \"" << type_name(desc) << L"\"" << Endl;
		return 0;
	}

	return body;
}

Ref< Joint > PhysicsManagerHavok::createJoint(const JointDesc* desc, const Transform& transform, Body* body1, Body* body2)
{
	T_ASSERT (desc);
	T_ASSERT (body1);

	HvkRef< hkpRigidBody > b1, b2;
	
	if (body1)
		b1 = checked_type_cast< BodyHavok* >(body1)->getRigidBody();
	if (body2)
		b2 = checked_type_cast< BodyHavok* >(body2)->getRigidBody();

	HvkRef< hkpConstraintInstance > constraint;
	Ref< Joint > joint;

	if (const BallJointDesc* ballDesc = dynamic_type_cast< const BallJointDesc* >(desc))
	{
		HvkRef< hkpBallAndSocketConstraintData > ballConstraintData;

		if (b1 && b2)
		{
			Vector4 anchor = transform * ballDesc->getAnchor().xyz1();
			Vector4 anchorIn1 = body1->getTransform().inverse() * anchor;
			Vector4 anchorIn2 = body2->getTransform().inverse() * anchor;

			ballConstraintData = new hkpBallAndSocketConstraintData();
			ballConstraintData->setInBodySpace(toHkVector4(anchorIn1), toHkVector4(anchorIn2));
		}
		else
		{
			Vector4 anchor = transform * ballDesc->getAnchor().xyz1();
			Vector4 anchorIn1 = body1->getTransform().inverse() * anchor;

			ballConstraintData = new hkpBallAndSocketConstraintData();
			ballConstraintData->setInBodySpace(
				toHkVector4(anchorIn1),
				toHkVector4(anchor)
			);
		}

		constraint = new hkpConstraintInstance(b1, b2, ballConstraintData);
		joint = new BallJointHavok(this, constraint, body1, body2);
	}
	else if (const ConeTwistJointDesc* coneTwistDesc = dynamic_type_cast< const ConeTwistJointDesc* >(desc))
	{
		HvkRef< hkpBallAndSocketConstraintData > ballConstraintData;

		if (b1 && b2)
		{
			Vector4 anchor = transform * coneTwistDesc->getAnchor().xyz1();
			Vector4 anchorIn1 = body1->getTransform().inverse() * anchor;
			Vector4 anchorIn2 = body2->getTransform().inverse() * anchor;

			ballConstraintData = new hkpBallAndSocketConstraintData();
			ballConstraintData->setInBodySpace(
				toHkVector4(anchorIn1),
				toHkVector4(anchorIn2)
			);
		}
		else
		{
			Vector4 anchor = transform * coneTwistDesc->getAnchor().xyz1();
			Vector4 anchorIn1 = body1->getTransform().inverse() * anchor;

			ballConstraintData = new hkpBallAndSocketConstraintData();
			ballConstraintData->setInBodySpace(
				toHkVector4(anchorIn1),
				toHkVector4(anchor)
			);
		}

		constraint = new hkpConstraintInstance(b1, b2, ballConstraintData);
		joint = new ConeTwistJointHavok(this, constraint, body1, body2);
	}
	else if (const HingeJointDesc* hingeDesc = dynamic_type_cast< const HingeJointDesc* >(desc))
	{
		HvkRef< hkpHingeConstraintData > hingeConstraintData;

		if (b1 && b2)
		{
			Vector4 anchor = transform * hingeDesc->getAnchor().xyz1();
			Vector4 axis = transform * hingeDesc->getAxis().xyz0();

			Vector4 anchorIn1 = body1->getTransform().inverse() * anchor;
			Vector4 anchorIn2 = body2->getTransform().inverse() * anchor;
			Vector4 axisIn1 = body1->getTransform().inverse() * axis;
			Vector4 axisIn2 = body2->getTransform().inverse() * axis;

			hingeConstraintData = new hkpHingeConstraintData();
			hingeConstraintData->setInBodySpace(
				toHkVector4(anchorIn1),
				toHkVector4(anchorIn2),
				toHkVector4(axisIn1),
				toHkVector4(axisIn2)
			);
		}
		else
		{
			Vector4 anchor = transform * hingeDesc->getAnchor().xyz1();
			Vector4 axis = transform * hingeDesc->getAxis().xyz0();

			Vector4 anchorIn1 = body1->getTransform().inverse() * anchor;
			Vector4 axisIn1 = body1->getTransform().inverse() * axis;

			hingeConstraintData = new hkpHingeConstraintData();
			hingeConstraintData->setInBodySpace(
				toHkVector4(anchorIn1),
				toHkVector4(anchor),
				toHkVector4(axisIn1),
				toHkVector4(axis)
			);
		}

		constraint = new hkpConstraintInstance(b1, b2, hingeConstraintData);
		joint = new HingeJointHavok(this, constraint, body1, body2);
	}

	if (!joint)
	{
		log::error << L"Unable to create joint, unknown joint type \"" << type_name(desc) << L"\"" << Endl;
		return 0;
	}

	if (constraint)
		m_world->addConstraint(constraint);

	m_joints.push_back(joint);

	return joint;
}

void PhysicsManagerHavok::update(bool issueCollisionEvents)
{
	T_ASSERT (m_world);
	m_world->stepDeltaTime(m_simulationDeltaTime);
}

RefArray< Body > PhysicsManagerHavok::getBodies() const
{
	return (RefArray< Body >&)m_bodies;
}

uint32_t PhysicsManagerHavok::getCollidingPairs(std::vector< CollisionPair >& outCollidingPairs) const
{
	return 0;
}

bool PhysicsManagerHavok::queryPoint(
	const Vector4& at,
	float margin,
	QueryResult& outResult
) const
{
	return false;
}

bool PhysicsManagerHavok::queryRay(
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	uint32_t group,
	const Body* ignoreBody,
	bool ignoreBackFace,
	QueryResult& outResult
) const
{
	T_ASSERT (m_world);

	hkpWorldRayCastInput input;
	input.m_from = toHkVector4(at);
	input.m_to = toHkVector4(at + direction * Scalar(maxLength));
	input.m_enableShapeCollectionFilter = false;

	if (ignoreBody)
	{
		IgnoreBodyClosestRayHitCollector collector(
			static_cast< const BodyHavok* >(ignoreBody)->getRigidBody()->getCollidableRw()
		);

		m_world->castRay(input, collector);
		if (!collector.hasHit())
			return false;

		const hkpWorldRayCastOutput& hit = collector.getHit();

		outResult.distance = hit.m_hitFraction * maxLength;
		outResult.position = at + direction * Scalar(outResult.distance);
		outResult.normal = fromHkVector4(hit.m_normal);
	}
	else
	{
		hkpClosestRayHitCollector collector;

		m_world->castRay(input, collector);
		if (!collector.hasHit())
			return false;

		const hkpWorldRayCastOutput& hit = collector.getHit();

		outResult.distance = hit.m_hitFraction * maxLength;
		outResult.position = at + direction * Scalar(outResult.distance);
		outResult.normal = fromHkVector4(hit.m_normal);
	}

	return true;
}

uint32_t PhysicsManagerHavok::querySphere(
	const Vector4& at,
	float radius,
	uint32_t group,
	uint32_t queryTypes,
	RefArray< Body >& outBodies
) const
{
	outBodies.resize(0);

	if (queryTypes & QtStatic)
	{
		for (RefArray< BodyHavok >::const_iterator i = m_bodies.begin(); i != m_bodies.end(); ++i)
		{
			if (!(*i)->isStatic())
				continue;

			hkpRigidBody* rigidBody = (*i)->getRigidBody();
			T_ASSERT (rigidBody);

			hkAabb aabb;
			rigidBody->getCollidable()->getShape()->getAabb(
				rigidBody->getTransform(),
				0.0f,
				aabb
			);

			Vector4 mn = fromHkVector4(aabb.m_min).xyz1();
			Vector4 mx = fromHkVector4(aabb.m_max).xyz1();

			float bodyRadius = (mx - mn).length() * 0.5f;
			Vector4 bodyCenter = (mn + mx) * Scalar(0.5f);

			if ((bodyCenter - at).length() - radius - bodyRadius <= 0.0f)
				outBodies.push_back(*i);
		}
	}

	if (queryTypes & QtDynamic)
	{
		for (RefArray< BodyHavok >::const_iterator i = m_bodies.begin(); i != m_bodies.end(); ++i)
		{
			if ((*i)->isStatic())
				continue;

			hkpRigidBody* rigidBody = (*i)->getRigidBody();
			T_ASSERT (rigidBody);

			hkAabb aabb;
			rigidBody->getCollidable()->getShape()->getAabb(
				rigidBody->getTransform(),
				0.0f,
				aabb
			);

			Vector4 mn = fromHkVector4(aabb.m_min).xyz1();
			Vector4 mx = fromHkVector4(aabb.m_max).xyz1();

			float bodyRadius = (mx - mn).length() * 0.5f;
			Vector4 bodyCenter = (mn + mx) * Scalar(0.5f);

			if ((bodyCenter - at).length() - radius - bodyRadius <= 0.0f)
				outBodies.push_back(*i);
		}
	}

	return uint32_t(outBodies.size());
}

bool PhysicsManagerHavok::querySweep(
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	float radius,
	uint32_t group,
	const Body* ignoreBody,
	QueryResult& outResult
) const
{
	hkTransform sphereTransform;
	sphereTransform.setIdentity();
	sphereTransform.setTranslation(toHkVector4(at));

	hkpSphereShape sphereShape(radius);
	hkpCollidable sphereColl(&sphereShape, &sphereTransform);

	hkpLinearCastInput li;
	li.m_to = toHkVector4(at + direction * Scalar(maxLength));

	if (ignoreBody)
	{
		IgnoreBodyClosestContactCollector collector(
			static_cast< const BodyHavok* >(ignoreBody)->getRigidBody()->getCollidableRw()
		);

		m_world->linearCast(&sphereColl, li, collector);
		if (!collector.hasHit())
			return false;

		const hkContactPoint& hit = collector.getHitContact();

		outResult.body = 0;
		outResult.position = fromHkVector4(hit.getPosition());
		outResult.normal = fromHkVector4(hit.getNormal());
		outResult.distance = hit.getDistance() + radius;
	}
	else
	{
		hkpSimpleClosestContactCollector collector;

		m_world->linearCast(&sphereColl, li, collector);
		if (!collector.hasHit())
			return false;

		const hkContactPoint& hit = collector.getHitContact();

		outResult.body = 0;
		outResult.position = fromHkVector4(hit.getPosition());
		outResult.normal = fromHkVector4(hit.getNormal());
		outResult.distance = hit.getDistance() + radius;
	}

	return true;
}

bool PhysicsManagerHavok::queryShadowRay(
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	uint32_t group,
	uint32_t queryTypes,
	const Body* ignoreBody
) const
{
	return false;
}

bool PhysicsManagerHavok::querySweep(
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
	return false;
}

void PhysicsManagerHavok::querySweep(
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	float radius,
	uint32_t group,
	const Body* ignoreBody,
	RefArray< Body >& outResult
) const
{
}

void PhysicsManagerHavok::queryOverlap(
	const Body* body,
	RefArray< Body >& outResult
) const
{
}

void PhysicsManagerHavok::getBodyCount(uint32_t& outCount, uint32_t& outActiveCount) const
{
	outCount = uint32_t(m_bodies.size());

	outActiveCount = 0;
	for (RefArray< BodyHavok >::const_iterator i = m_bodies.begin(); i != m_bodies.end(); ++i)
	{
		if ((*i)->isActive())
			++outActiveCount;
	}
}

void PhysicsManagerHavok::destroyBody(Body* body, const HvkRef< hkpRigidBody >& rigidBody)
{
	m_world->removeEntity(rigidBody);

	RefArray< BodyHavok >::iterator i = std::find(m_bodies.begin(), m_bodies.end(), body);
	if (i != m_bodies.end())
		m_bodies.erase(i);
}

void PhysicsManagerHavok::destroyJoint(Joint* joint, const HvkRef< hkpConstraintInstance >& constraint)
{
	m_world->removeConstraint(constraint);

	RefArray< Joint >::iterator i = std::find(m_joints.begin(), m_joints.end(), joint);
	if (i != m_joints.end())
		m_joints.erase(i);
}

void PhysicsManagerHavok::showError(const char* error, void*)
{
	log::error << mbstows(error);
}

	}
}
