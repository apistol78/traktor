#include <algorithm>

// PhysX
#include <PxPhysicsAPI.h>
#include <extensions\PxExtensionsAPI.h>
#include <extensions\PxDefaultErrorCallback.h>
#include <extensions\PxDefaultAllocator.h>
#include <extensions\PxDefaultSimulationFilterShader.h>
#include <extensions\PxDefaultCpuDispatcher.h>
#include <extensions\PxShapeExt.h>
#include <extensions\PxSimpleFactory.h>
#include <foundation\PxFoundation.h>

// Traktor
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Log/Log.h"
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
#include "Physics/PhysX/BallJointPhysX.h"
#include "Physics/PhysX/BodyPhysX.h"
#include "Physics/PhysX/ConeTwistJointPhysX.h"
#include "Physics/PhysX/Conversion.h"
#include "Physics/PhysX/HingeJointPhysX.h"
#include "Physics/PhysX/PhysicsManagerPhysX.h"
#include "Physics/PhysX/PxMemoryStreams.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace physics
	{
		namespace
		{

physx::PxDefaultErrorCallback g_defaultErrorCallback;
physx::PxDefaultAllocator g_defaultAllocatorCallback;

physx::PxFilterFlags collisionFilterShader(	
	physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0, 
	physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
	physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
{
	pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT;

	if ((filterData0.word0 & filterData1.word1) || (filterData1.word0 & filterData0.word1))
		pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;

	return physx::PxFilterFlag::eDEFAULT;
}

class IgnoreBodyFilter : public physx::PxSceneQueryFilterCallback
{
public:
	IgnoreBodyFilter(uint32_t ignoreClusterId)
	:	m_ignoreClusterId(ignoreClusterId)
	{
	}

	virtual physx::PxSceneQueryHitType::Enum preFilter(const physx::PxFilterData& filterData, physx::PxShape* shape, physx::PxSceneQueryFilterFlags& filterFlags)
	{
		const Body* body = reinterpret_cast< const Body* >(shape->userData);
		if (body->getClusterId() == m_ignoreClusterId)
			return physx::PxSceneQueryHitType::eNONE;
		else
			return physx::PxSceneQueryHitType::eTOUCH;
	}

	virtual physx::PxSceneQueryHitType::Enum postFilter(const physx::PxFilterData& filterData, const physx::PxSceneQueryHit& hit)
	{
		return physx::PxSceneQueryHitType::eTOUCH;
	}

private:
	uint32_t m_ignoreClusterId;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.PhysicsManagerPhysX", 0, PhysicsManagerPhysX, PhysicsManager)

PhysicsManagerPhysX::PhysicsManagerPhysX()
:	m_simulationDeltaTime(0.0f)
,	m_sdk(0)
,	m_cooking(0)
,	m_scene(0)
{
}

PhysicsManagerPhysX::~PhysicsManagerPhysX()
{
	destroy();
}

bool PhysicsManagerPhysX::create(float simulationDeltaTime, float timeScale)
{
	physx::PxFoundation* foundation = PxCreateFoundation(PX_PHYSICS_VERSION, g_defaultAllocatorCallback, g_defaultErrorCallback);
	if (!foundation) 
	{
		log::error << L"Unable to create PhysX foundation" << Endl;
		return false;
	}

	m_sdk = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, physx::PxTolerancesScale());
	if (!m_sdk) 
	{
		log::error << L"Unable to initialize the PhysX SDK" << Endl;
		return false;
	}

	m_cooking = PxCreateCooking(PX_PHYSICS_VERSION, *foundation, physx::PxCookingParams());

	if (m_sdk->getPvdConnectionManager())
	{
		const char* pvdHostIP = "127.0.0.1";
		const int port = 5425;
		const unsigned int timeout = 100;

		physx::PxVisualDebuggerConnectionFlags connectionFlags =
			physx::PxVisualDebuggerConnectionFlag::Debug |
			physx::PxVisualDebuggerConnectionFlag::Profile |
			physx::PxVisualDebuggerConnectionFlag::Memory;

		PVD::PvdConnection* connection = physx::PxVisualDebuggerExt::createConnection(
			m_sdk->getPvdConnectionManager(),
			pvdHostIP,
			port,
			timeout,
			connectionFlags
		);
		if (!connection)
			log::warning << L"Unable to connect to PhysX PVD" << Endl;
	}

	physx::PxSceneDesc sceneDesc(m_sdk->getTolerancesScale());
	sceneDesc.gravity = physx::PxVec3(0.0f, -9.8f, 0.0f);
	sceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(1);
	sceneDesc.filterShader = collisionFilterShader;

	m_scene = m_sdk->createScene(sceneDesc);
	if (!m_scene) 
	{
		log::error << L"Unable to create a PhysX scene" << Endl;
		return false;
	}

	m_simulationDeltaTime = simulationDeltaTime;
	return true;
}

void PhysicsManagerPhysX::destroy()
{
	if (m_scene)
	{
		T_ASSERT (m_sdk);
		m_scene->release();
		m_scene = 0;
	}
	if (m_cooking)
	{
		T_ASSERT (m_sdk);
		m_cooking->release();
		m_cooking = 0;
	}
	if (m_sdk)
	{
		m_sdk->release();
		m_sdk = 0;
	}
}

void PhysicsManagerPhysX::setGravity(const Vector4& gravity)
{
	m_scene->setGravity(toPxVec3(gravity));
}

Vector4 PhysicsManagerPhysX::getGravity() const
{
	return fromPxVec3(m_scene->getGravity(), 0.0f);
}

Ref< Body > PhysicsManagerPhysX::createBody(resource::IResourceManager* resourceManager, const BodyDesc* desc)
{
	if (!desc)
		return 0;

	const ShapeDesc* shapeDesc = desc->getShape();
	if (!shapeDesc)
	{
		log::error << L"Unable to create body, no shape defined" << Endl;
		return 0;
	}

	physx::PxGeometry* geometry = 0;
	Vector4 centerOfGravity = Vector4::origo();

	if (const BoxShapeDesc* boxShape = dynamic_type_cast< const BoxShapeDesc* >(shapeDesc))
	{
		physx::PxBoxGeometry* boxGeometry = new physx::PxBoxGeometry(
			toPxVec3(boxShape->getExtent())
		);
		geometry = boxGeometry;
	}
	else if (const CapsuleShapeDesc* capsuleShape = dynamic_type_cast< const CapsuleShapeDesc* >(shapeDesc))
	{
		float radius = capsuleShape->getRadius();
		float length = capsuleShape->getLength() - radius * 2.0f;
		physx::PxCapsuleGeometry* capsuleGeometry = new physx::PxCapsuleGeometry(
			radius,
			length
		);
		geometry = capsuleGeometry;
	}
	else if (const CylinderShapeDesc* cylinderShape = dynamic_type_cast< const CylinderShapeDesc* >(shapeDesc))
	{
		float radius = cylinderShape->getRadius();
		float length = cylinderShape->getLength() - radius * 2.0f;
		physx::PxCapsuleGeometry* capsuleGeometry = new physx::PxCapsuleGeometry(
			radius,
			length
		);
		geometry = capsuleGeometry;
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
		const std::vector< Mesh::Triangle >& hullTriangles = mesh->getHullTriangles();

		physx::PxTriangleMeshDesc meshDesc;
		meshDesc.points.count = vertices.size();
		meshDesc.points.stride = sizeof(Vector4);
		meshDesc.points.data = vertices.c_ptr();
		meshDesc.triangles.count = shapeTriangles.size();
		meshDesc.triangles.stride = sizeof(Mesh::Triangle);
		meshDesc.triangles.data = &shapeTriangles[0];
		meshDesc.flags = physx::PxMeshFlag::eFLIPNORMALS;

		MemoryOutputStream writeBuffer;
		bool status = m_cooking->cookTriangleMesh(meshDesc, writeBuffer);
		if( !status)
			return 0;

		MemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
		physx::PxTriangleMesh* triangleMesh = m_sdk->createTriangleMesh(readBuffer);
		if (!triangleMesh)
			return 0;

		physx::PxTriangleMeshGeometry* triangleMeshGeometry = new physx::PxTriangleMeshGeometry(
			triangleMesh
		);
		geometry = triangleMeshGeometry;

		centerOfGravity = mesh->getOffset();
	}
	else if (const SphereShapeDesc* sphereShape = dynamic_type_cast< const SphereShapeDesc* >(shapeDesc))
	{
		float radius = sphereShape->getRadius();
		physx::PxSphereGeometry* sphereGeometry = new physx::PxSphereGeometry(
			radius
		);
		geometry = sphereGeometry;
	}
	else if (const HeightfieldShapeDesc* heightfieldShape = dynamic_type_cast< const HeightfieldShapeDesc* >(shapeDesc))
	{
		resource::Proxy< hf::Heightfield > heightfield;
		if (resourceManager->bind(heightfieldShape->getHeightfield(), heightfield))
		{
			log::error << L"Unable to load heightfield resource" << Endl;
			return 0;
		}

		const hf::height_t* heights = heightfield->getHeights();
		T_ASSERT (heights);

		physx::PxU8* pxh = new physx::PxU8 [heightfield->getSize() * heightfield->getSize()];
		physx::PxU8* ptr = pxh;

		for (int32_t z = 0; z < heightfield->getSize(); ++z)
		{
			for (int32_t x = 0; x < heightfield->getSize(); ++x)
			{
				hf::height_t height = heights[z + x * heightfield->getSize()];

				*(physx::PxI16*)ptr = physx::PxI16(int32_t(height) - 32767);
				ptr[2] = 0;
				ptr[3] = 0;

				ptr += 4;
			}
		}

		physx::PxHeightFieldDesc hfDesc;
		hfDesc.format = physx::PxHeightFieldFormat::eS16_TM;
		hfDesc.nbColumns = heightfield->getSize();
		hfDesc.nbRows = heightfield->getSize();
		hfDesc.samples.data = pxh;
		hfDesc.samples.stride = sizeof(physx::PxU32);

		physx::PxHeightField* pxhf = m_sdk->createHeightField(hfDesc);
		T_ASSERT (pxhf);

		physx::PxHeightFieldGeometry* heightfieldGeometry = new physx::PxHeightFieldGeometry(
			pxhf,
			physx::PxMeshGeometryFlags(0),
			heightfield->getWorldExtent().y() / 65535.0f,
			heightfield->getWorldExtent().x() / heightfield->getSize(),
			heightfield->getWorldExtent().z() / heightfield->getSize()
		);
		geometry = heightfieldGeometry;
	}
	
	if (!geometry)
	{
		log::error << L"Unsupported shape type \"" << type_name(shapeDesc) << L"\"" << Endl;
		return 0;
	}

	if (shapeDesc->getLocalTransform() != Transform::identity())
	{
	}

	Ref< Body > body;

	if (const StaticBodyDesc* staticDesc = dynamic_type_cast< const StaticBodyDesc* >(desc))
	{
		physx::PxRigidStatic* rigidBody = m_sdk->createRigidStatic(toPxTransform(Transform::identity()));
		if (!rigidBody)
		{
			log::error << L"Unable to create PhysX rigid body" << Endl;
			return 0;
		}

		physx::PxMaterial* material = m_sdk->createMaterial(staticDesc->getFriction(), staticDesc->getFriction(), 0.1f);
		if (!material)
			return 0;

		physx::PxShape* shape = rigidBody->createShape(*geometry, *material);
		T_ASSERT (shape);

		physx::PxFilterData filterData;
		filterData.word0 = shapeDesc->getCollisionGroup();
		filterData.word1 = shapeDesc->getCollisionMask();
		shape->setSimulationFilterData(filterData);
		shape->setQueryFilterData(filterData);

		Ref< BodyPhysX > staticBody = new BodyPhysX(this, m_scene, rigidBody, centerOfGravity);
		m_bodies.push_back(staticBody);

		shape->userData = staticBody;
		body = staticBody;
	}
	else if (const DynamicBodyDesc* dynamicDesc = dynamic_type_cast< const DynamicBodyDesc* >(desc))
	{
		physx::PxRigidDynamic* rigidBody = m_sdk->createRigidDynamic(toPxTransform(Transform::identity()));
		if (!rigidBody)
		{
			log::error << L"Unable to create PhysX actor" << Endl;
			return 0;
		}

		rigidBody->setLinearDamping(dynamicDesc->getLinearDamping());
		rigidBody->setAngularDamping(dynamicDesc->getAngularDamping());

		physx::PxMaterial* material = m_sdk->createMaterial(dynamicDesc->getFriction(), dynamicDesc->getFriction(), 0.1f);
		if (!material)
			return 0;

		physx::PxShape* shape = rigidBody->createShape(*geometry, *material);
		T_ASSERT (shape);

		physx::PxFilterData filterData;
		filterData.word0 = shapeDesc->getCollisionGroup();
		filterData.word1 = shapeDesc->getCollisionMask();
		shape->setSimulationFilterData(filterData);
		shape->setQueryFilterData(filterData);

		physx::PxRigidBodyExt::setMassAndUpdateInertia(*rigidBody, dynamicDesc->getMass());

		if (!dynamicDesc->getActive())
			rigidBody->putToSleep();

		Ref< BodyPhysX > dynamicBody = new BodyPhysX(this, m_scene, rigidBody, centerOfGravity);
		m_bodies.push_back(dynamicBody);

		shape->userData = dynamicBody;
		body = dynamicBody;
	}
	else
	{
		log::error << L"Unsupported body type \"" << type_name(desc) << L"\"" << Endl;
		return 0;
	}

	delete geometry;
	return body;
}

Ref< Joint > PhysicsManagerPhysX::createJoint(const JointDesc* desc, const Transform& transform, Body* body1, Body* body2)
{
	physx::PxRigidActor* actor1 = body1 ? checked_type_cast< BodyPhysX* >(body1)->getPxRigidActor() : 0;
	physx::PxRigidActor* actor2 = body2 ? checked_type_cast< BodyPhysX* >(body2)->getPxRigidActor() : 0;

	Ref< Joint > outJoint;

	if (const BallJointDesc* ballDesc = dynamic_type_cast< const BallJointDesc* >(desc))
	{
		Vector4 anchorW = transform * ballDesc->getAnchor().xyz1();

		Transform local1 = body1->getTransform().inverse() * Transform(anchorW);
		Transform local2 = body2->getTransform().inverse() * Transform(anchorW);

		physx::PxSphericalJoint* sphereJoint = physx::PxSphericalJointCreate(
			*m_sdk,
			actor1,
			toPxTransform(local1),
			actor2,
			toPxTransform(local2)
		);

		outJoint = new BallJointPhysX(this, sphereJoint, body1, body2);
	}
	/*
	else if (const ConeTwistJointDesc* coneTwistDesc = dynamic_type_cast< const ConeTwistJointDesc* >(desc))
	{
		float coneAngle1, coneAngle2;
		coneTwistDesc->getConeAngles(coneAngle1, coneAngle2);

		NxD6JointDesc jointDesc;
		jointDesc.actor[0] = actor1;
		jointDesc.actor[1] = actor2;
		jointDesc.xMotion = NX_D6JOINT_MOTION_LOCKED;
		jointDesc.yMotion = NX_D6JOINT_MOTION_LOCKED;
		jointDesc.zMotion = NX_D6JOINT_MOTION_LOCKED;
		jointDesc.swing1Motion = NX_D6JOINT_MOTION_LIMITED;
		jointDesc.swing2Motion = NX_D6JOINT_MOTION_LIMITED;
		jointDesc.twistMotion = NX_D6JOINT_MOTION_LIMITED;
		jointDesc.swing1Limit.value = coneAngle1;
		jointDesc.swing2Limit.value = coneAngle2;
		jointDesc.twistLimit.low.value = -coneTwistDesc->getTwistAngle();
		jointDesc.twistLimit.high.value = coneTwistDesc->getTwistAngle();
		jointDesc.setGlobalAnchor(toPxVec3(transform * coneTwistDesc->getAnchor().xyz1()));
		jointDesc.setGlobalAxis(toPxVec3(transform * coneTwistDesc->getTwistAxis().xyz0()));

		NxJoint* joint = m_scene->createJoint(jointDesc);
		if (!joint)
			return 0;

		outJoint = new ConeTwistJointPhysX(this, joint, body1, body2);
	}
	*/
	else if (const HingeJointDesc* hingeDesc = dynamic_type_cast< const HingeJointDesc* >(desc))
	{
		Vector4 anchorW = transform * hingeDesc->getAnchor().xyz1();
		Vector4 axisW = transform * hingeDesc->getAxis().xyz0();

		Transform local1 = body1->getTransform().inverse() * Transform(anchorW, Quaternion(Vector4(1.0f, 0.0f, 0.0f), axisW));
		Transform local2 = body2->getTransform().inverse() * Transform(anchorW, Quaternion(Vector4(1.0f, 0.0f, 0.0f), axisW));

		physx::PxRevoluteJoint* revoluteJoint = physx::PxRevoluteJointCreate(
			*m_sdk,
			actor1,
			toPxTransform(local1),
			actor2,
			toPxTransform(local2)
		);

		outJoint = new HingeJointPhysX(this, revoluteJoint, body1, body2);
	}
	else
	{
		log::error << L"Unsupported joint type \"" << type_name(desc) << L"\"" << Endl;
		return 0;
	}

	return outJoint;
}

void PhysicsManagerPhysX::update(bool issueCollisionEvents)
{
	m_scene->simulate(m_simulationDeltaTime);
	m_scene->fetchResults(true);
}

void PhysicsManagerPhysX::solveConstraints(const RefArray< Body >& bodies, const RefArray< Joint >& joints)
{
}

RefArray< Body > PhysicsManagerPhysX::getBodies() const
{
	return (RefArray< Body >&)m_bodies;
}

uint32_t PhysicsManagerPhysX::getCollidingPairs(std::vector< CollisionPair >& outCollidingPairs) const
{
	return 0;
}

bool PhysicsManagerPhysX::queryPoint(
	const Vector4& at,
	float margin,
	QueryResult& outResult
) const
{
	return false;
}

bool PhysicsManagerPhysX::queryRay(
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	uint32_t group,
	uint32_t ignoreClusterId,
	bool ignoreBackFace,
	QueryResult& outResult
) const
{
	physx::PxRaycastHit hit;

	if (ignoreClusterId == 0)
	{
		physx::PxSceneQueryFilterData filterData;
		filterData.data.word0 = group;
		filterData.flags = physx::PxSceneQueryFilterFlag::eDYNAMIC | physx::PxSceneQueryFilterFlag::eSTATIC;

		if (!m_scene->raycastSingle(
			toPxVec3(at),
			toPxVec3(direction),
			maxLength,
			physx::PxSceneQueryFlag::eIMPACT | physx::PxSceneQueryFlag::eNORMAL | physx::PxSceneQueryFlag::eDISTANCE,
			hit,
			filterData
		))
			return false;
	}
	else
	{
		IgnoreBodyFilter filter(ignoreClusterId);

		physx::PxSceneQueryFilterData filterData;
		filterData.data.word0 = group;
		filterData.flags = physx::PxSceneQueryFilterFlag::eDYNAMIC | physx::PxSceneQueryFilterFlag::eSTATIC | physx::PxSceneQueryFilterFlag::ePREFILTER;

		if (!m_scene->raycastSingle(
			toPxVec3(at),
			toPxVec3(direction),
			maxLength,
			physx::PxSceneQueryFlag::eIMPACT | physx::PxSceneQueryFlag::eNORMAL | physx::PxSceneQueryFlag::eDISTANCE,
			hit,
			filterData,
			&filter
		))
			return false;
	}

	outResult.distance = hit.distance;
	outResult.position = fromPxVec3(hit.impact, 1.0f);
	outResult.normal = fromPxVec3(hit.normal, 0.0f);
	outResult.body = reinterpret_cast< Body* >(hit.shape->userData);

	return true;
}

bool PhysicsManagerPhysX::queryShadowRay(
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	uint32_t group,
	uint32_t queryTypes,
	uint32_t ignoreClusterId
) const
{
	physx::PxRaycastHit hit;
	return m_scene->raycastAny(
		toPxVec3(at),
		toPxVec3(direction),
		maxLength,
		hit
	);
}

uint32_t PhysicsManagerPhysX::querySphere(
	const Vector4& at,
	float radius,
	uint32_t group,
	uint32_t queryTypes,
	RefArray< Body >& outBodies
) const
{
	return 0;
}

bool PhysicsManagerPhysX::querySweep(
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	float radius,
	uint32_t group,
	uint32_t ignoreClusterId,
	QueryResult& outResult
) const
{
	return false;
}

bool PhysicsManagerPhysX::querySweep(
	const Body* body,
	const Quaternion& orientation,
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	uint32_t group,
	uint32_t ignoreClusterId,
	QueryResult& outResult
) const
{
	return false;
}

void PhysicsManagerPhysX::querySweep(
	const Vector4& at,
	const Vector4& direction,
	float maxLength,
	float radius,
	uint32_t group,
	uint32_t ignoreClusterId,
	RefArray< Body >& outResult
) const
{
}

void PhysicsManagerPhysX::queryOverlap(
	const Body* body,
	RefArray< Body >& outResult
) const
{
}

void PhysicsManagerPhysX::getStatistics(PhysicsStatistics& outStatistics) const
{
	outStatistics.bodyCount = m_bodies.size();
	outStatistics.activeCount = 0;
	outStatistics.manifoldCount = 0;
	outStatistics.queryCount = 0;
}

void PhysicsManagerPhysX::destroyBody(Body* owner, physx::PxRigidActor* actor)
{
	RefArray< BodyPhysX >::iterator i = std::find(m_bodies.begin(), m_bodies.end(), owner);
	if (i != m_bodies.end())
		m_bodies.erase(i);
	actor->release();
}

void PhysicsManagerPhysX::destroyJoint(Joint* owner, physx::PxJoint* joint)
{
	joint->release();
}

	}
}
