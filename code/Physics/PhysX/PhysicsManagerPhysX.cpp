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
#include "Core/Containers/StaticVector.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Math/Aabb3.h"
#include "Core/Misc/TString.h"
#include "Core/Thread/Acquire.h"
#include "Heightfield/Heightfield.h"
#include "Physics/BallJointDesc.h"
#include "Physics/BoxShapeDesc.h"
#include "Physics/CapsuleShapeDesc.h"
#include "Physics/ConeTwistJointDesc.h"
#include "Physics/CylinderShapeDesc.h"
#include "Physics/DynamicBodyDesc.h"
#include "Physics/FixedJointDesc.h"
#include "Physics/HeightfieldShapeDesc.h"
#include "Physics/HingeJointDesc.h"
#include "Physics/Hinge2JointDesc.h"
#include "Physics/Mesh.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/SphereShapeDesc.h"
#include "Physics/StaticBodyDesc.h"
#include "Physics/PhysX/BallJointPhysX.h"
#include "Physics/PhysX/BodyPhysX.h"
#include "Physics/PhysX/ConeTwistJointPhysX.h"
#include "Physics/PhysX/Conversion.h"
#include "Physics/PhysX/FixedJointPhysX.h"
#include "Physics/PhysX/HingeJointPhysX.h"
#include "Physics/PhysX/Hinge2JointPhysX.h"
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

	// Let triggers through.
	if(physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
	{
		pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
		return physx::PxFilterFlags();
	}

	BodyPhysX* body0 = reinterpret_cast< BodyPhysX* >(filterData0.word2);
	BodyPhysX* body1 = reinterpret_cast< BodyPhysX* >(filterData1.word2);

	uint32_t cluster0 = body0->getClusterId();
	if (cluster0 != ~0UL)
	{
		uint32_t cluster1 = body1->getClusterId();
		if (cluster0 == cluster1)
		{
			// No contact.
			return physx::PxFilterFlag::eSUPPRESS;
		}
	}

	// Filter collision on collision group and mask.
	uint32_t group1 = filterData0.word0;
	uint32_t mask1 = filterData0.word1;
	uint32_t group2 = filterData1.word0;
	uint32_t mask2 = filterData1.word1;

	if ((group1 & mask2) == 0 || (group2 & mask1) == 0)
	{
		// No contact.
		return physx::PxFilterFlag::eSUPPRESS;
	}

	// Should issue collisions.
	pairFlags = physx::PxPairFlag::eCONTACT_DEFAULT; // | physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
	return physx::PxFilterFlags();
}

class EventCallback : public physx::PxSimulationEventCallback
{
public:
	EventCallback(AlignedVector< CollisionInfo >& outCollisionInfo)
	:	m_outCollisionInfo(outCollisionInfo)
	{
	}

	virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count)
	{
	}

	virtual void onWake(physx::PxActor** actors, physx::PxU32 count)
	{
	}

	virtual void onSleep(physx::PxActor** actors, physx::PxU32 count)
	{
	}

	virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
	{
		for (physx::PxU32 i = 0; i < nbPairs; ++i)
		{
			BodyPhysX* body1 = reinterpret_cast< BodyPhysX* >(pairs[i].shapes[0]->userData);
			BodyPhysX* body2 = reinterpret_cast< BodyPhysX* >(pairs[i].shapes[1]->userData);

			//CollisionInfo& info = m_outCollisionInfo.push_back();
			//info.body1 = body1;
			//info.body2 = body2;
			//info.contacts.resize(0);
			//info.contacts.reserve(contacts);

			//int32_t material1 = body1->getMaterial();
			//int32_t material2 = body2->getMaterial();

			//for (int32_t j = 0; j < contacts; ++j)
			//{
			//	const btManifoldPoint& pt = manifold->getContactPoint(j);
			//	if (pt.getDistance() < 0.0f)
			//	{
			//		CollisionContact cc;
			//		cc.depth = -pt.getDistance();
			//		cc.normal = fromBtVector3(pt.m_normalWorldOnB, 0.0f);
			//		cc.position = fromBtVector3(pt.m_positionWorldOnA, 1.0f);
			//		cc.material1 = material1;
			//		cc.material2 = material2;
			//		info.contacts.push_back(cc);
			//	}
			//}
		}
	}

	virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
	{
	}

private:
	AlignedVector< CollisionInfo >& m_outCollisionInfo;
};

class IgnoreBodyFilter : public physx::PxQueryFilterCallback
{
public:
	IgnoreBodyFilter(uint32_t ignoreClusterId, uint32_t group)
	:	m_ignoreClusterId(ignoreClusterId)
	,	m_group(group)
	{
	}

	virtual physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags)
	{
		const BodyPhysX* body = reinterpret_cast< const BodyPhysX* >(shape->userData);
		if (body->getClusterId() == m_ignoreClusterId || (body->getCollisionGroup() & m_group) == 0)
			return physx::PxQueryHitType::eNONE;
		else
			return physx::PxQueryHitType::eBLOCK;
	}

	virtual physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit)
	{
		return physx::PxQueryHitType::eTOUCH;
	}

private:
	uint32_t m_ignoreClusterId;
	uint32_t m_group;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.PhysicsManagerPhysX", 0, PhysicsManagerPhysX, PhysicsManager)

PhysicsManagerPhysX::PhysicsManagerPhysX()
:	m_simulationDeltaTime(0.0f)
,	m_timeScale(1.0f)
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
	log::info << L"Initializing PhysX " << PX_PHYSICS_VERSION_MAJOR << L"." << PX_PHYSICS_VERSION_MINOR << L"." << PX_PHYSICS_VERSION_BUGFIX << L"..." << Endl;

	physx::PxFoundation* foundation = PxCreateFoundation(PX_PHYSICS_VERSION, g_defaultAllocatorCallback, g_defaultErrorCallback);
	if (!foundation) 
	{
		log::error << L"Unable to create PhysX foundation" << Endl;
		return false;
	}

	physx::PxTolerancesScale toleranceScale;

	m_sdk = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, toleranceScale);
	if (!m_sdk) 
	{
		log::error << L"Unable to initialize the PhysX SDK" << Endl;
		return false;
	}

	if (!PxInitExtensions(*m_sdk))
	{
		log::error << L"Unable to initialize PhysX extensions" << Endl;
		return false;
	}

	m_cooking = PxCreateCooking(PX_PHYSICS_VERSION, *foundation, physx::PxCookingParams(toleranceScale));

	if (m_sdk->getPvdConnectionManager())
	{
		const char* pvdHostIP = "127.0.0.1";
		const int port = 5425;
		const unsigned int timeout = 100;

		physx::PxVisualDebuggerConnectionFlags connectionFlags =
			physx::PxVisualDebuggerConnectionFlag::eDEBUG |
			physx::PxVisualDebuggerConnectionFlag::ePROFILE |
			physx::PxVisualDebuggerConnectionFlag::eMEMORY;

		physx::PxVisualDebuggerConnection* connection = physx::PxVisualDebuggerExt::createConnection(
			m_sdk->getPvdConnectionManager(),
			pvdHostIP,
			port,
			timeout,
			connectionFlags
		);
		if (!connection)
			log::warning << L"Unable to connect to PhysX PVD" << Endl;

		m_sdk->getVisualDebugger()->setVisualDebuggerFlags(physx::PxVisualDebuggerFlag::eTRANSMIT_CONSTRAINTS | physx::PxVisualDebuggerFlag::eTRANSMIT_SCENEQUERIES);
	}

	physx::PxSceneDesc sceneDesc(toleranceScale);
	sceneDesc.gravity = physx::PxVec3(0.0f, -9.8f, 0.0f);
	sceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
	sceneDesc.filterShader = collisionFilterShader;
	sceneDesc.simulationEventCallback = new EventCallback(m_collisionInfo);

	m_scene = m_sdk->createScene(sceneDesc);
	if (!m_scene) 
	{
		log::error << L"Unable to create a PhysX scene" << Endl;
		return false;
	}

	m_simulationDeltaTime = simulationDeltaTime;
	m_timeScale = timeScale;

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

Ref< Body > PhysicsManagerPhysX::createBody(resource::IResourceManager* resourceManager, const BodyDesc* desc, const wchar_t* const tag)
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

	physx::PxGeometry* geometry = 0;
	physx::PxTransform shapePose = toPxTransform(shapeDesc->getLocalTransform());
	Vector4 centerOfGravity = Vector4::origo();
	float margin = 0.0f;

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
		const AlignedVector< Mesh::Triangle >& shapeTriangles = mesh->getShapeTriangles();
		const AlignedVector< Mesh::Triangle >& hullTriangles = mesh->getHullTriangles();

		if (is_a< DynamicBodyDesc >(desc))
		{
			physx::PxConvexMeshDesc convexDesc;
			convexDesc.points.count = vertices.size();
			convexDesc.points.stride = sizeof(Vector4);
			convexDesc.points.data = vertices.c_ptr();
			convexDesc.triangles.count = hullTriangles.size();
			convexDesc.triangles.stride = sizeof(Mesh::Triangle);
			convexDesc.triangles.data = &hullTriangles[0];
			convexDesc.flags = physx::PxConvexFlag::eFLIPNORMALS;

			PxMemoryOutputStream writeBuffer;
			bool status = m_cooking->cookConvexMesh(convexDesc, writeBuffer);
			if (!status)
			{
				log::error << L"Failed to create body; unable to cook convex collision shape." << Endl;
				log::error << L"\t" << int32_t(vertices.size()) << L" point(s)" << Endl;
				log::error << L"\t" << int32_t(hullTriangles.size()) << L" triangle(s)" << Endl;
				return 0;
			}

			PxMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
			physx::PxConvexMesh* convexMesh = m_sdk->createConvexMesh(readBuffer);
			if (!convexMesh)
			{
				log::error << L"Failed to create body; unable to create convex collision shape." << Endl;
				log::error << L"\t" << int32_t(vertices.size()) << L" point(s)" << Endl;
				log::error << L"\t" << int32_t(hullTriangles.size()) << L" triangle(s)" << Endl;
				return 0;
			}

			physx::PxConvexMeshGeometry* convexMeshGeometry = new physx::PxConvexMeshGeometry(
				convexMesh
			);
			geometry = convexMeshGeometry;
		}
		else
		{
			physx::PxTriangleMeshDesc meshDesc;
			meshDesc.points.count = vertices.size();
			meshDesc.points.stride = sizeof(Vector4);
			meshDesc.points.data = vertices.c_ptr();
			meshDesc.triangles.count = shapeTriangles.size();
			meshDesc.triangles.stride = sizeof(Mesh::Triangle);
			meshDesc.triangles.data = &shapeTriangles[0];
			meshDesc.flags = physx::PxMeshFlag::eFLIPNORMALS;

			PxMemoryOutputStream writeBuffer;
			bool status = m_cooking->cookTriangleMesh(meshDesc, writeBuffer);
			if (!status)
			{
				log::error << L"Failed to create body; unable to cook mesh collision shape." << Endl;
				log::error << L"\t" << int32_t(vertices.size()) << L" point(s)" << Endl;
				log::error << L"\t" << int32_t(shapeTriangles.size()) << L" triangle(s)" << Endl;
				return 0;
			}

			PxMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
			physx::PxTriangleMesh* triangleMesh = m_sdk->createTriangleMesh(readBuffer);
			if (!triangleMesh)
			{
				log::error << L"Failed to create body; unable to create mesh collision shape." << Endl;
				log::error << L"\t" << int32_t(vertices.size()) << L" point(s)" << Endl;
				log::error << L"\t" << int32_t(shapeTriangles.size()) << L" triangle(s)" << Endl;
				return 0;
			}

			physx::PxTriangleMeshGeometry* triangleMeshGeometry = new physx::PxTriangleMeshGeometry(
				triangleMesh
			);
			geometry = triangleMeshGeometry;
		}

		centerOfGravity = mesh->getOffset();
		margin = mesh->getMargin();
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
		if (!resourceManager->bind(heightfieldShape->getHeightfield(), heightfield))
		{
			log::error << L"Unable to load heightfield resource" << Endl;
			return 0;
		}

		const hf::height_t* heights = heightfield->getHeights();
		T_ASSERT (heights);

		int32_t hsize = heightfield->getSize();

		physx::PxHeightFieldSample* pxh = new physx::PxHeightFieldSample [hsize * hsize];
		physx::PxHeightFieldSample* ptr = pxh;

		std::memset(pxh, 0, hsize * hsize * sizeof(physx::PxHeightFieldSample));

		for (int32_t z = 0; z < hsize; ++z)
		{
			for (int32_t x = 0; x < hsize; ++x)
			{
				hf::height_t height = heights[z + x * hsize];

				ptr->height = physx::PxI16(int32_t(height) - 32767);

				if (heightfield->getGridCut(z, x))
				{
					ptr->materialIndex0 = 0;
					ptr->materialIndex1 = 0;
				}
				else
				{
					ptr->materialIndex0 = physx::PxHeightFieldMaterial::eHOLE;
					ptr->materialIndex1 = physx::PxHeightFieldMaterial::eHOLE;
				}

				ptr++;
			}
		}

		physx::PxHeightFieldDesc hfDesc;
		hfDesc.format = physx::PxHeightFieldFormat::eS16_TM;
		hfDesc.nbColumns = hsize;
		hfDesc.nbRows = hsize;
		hfDesc.samples.data = pxh;
		hfDesc.samples.stride = sizeof(physx::PxHeightFieldSample);
		hfDesc.thickness = -4.0f;
		hfDesc.convexEdgeThreshold = 1e8f;

		physx::PxHeightField* pxhf = m_cooking->createHeightField(hfDesc, m_sdk->getPhysicsInsertionCallback());
		T_ASSERT (pxhf);

		float sizeScaleX = heightfield->getWorldExtent().x() / hsize;
		float sizeScaleZ = heightfield->getWorldExtent().z() / hsize;

		physx::PxHeightFieldGeometry* heightfieldGeometry = new physx::PxHeightFieldGeometry(
			pxhf,
			physx::PxMeshGeometryFlags(),
			heightfield->getWorldExtent().y() / 65535.0f,
			sizeScaleX,
			sizeScaleZ
		);

		shapePose.p = physx::PxVec3(-(hsize / 2.0f * sizeScaleX), 0.0f ,-(hsize / 2.0f * sizeScaleZ));

		geometry = heightfieldGeometry;
	}
	
	if (!geometry)
	{
		log::error << L"Unsupported shape type \"" << type_name(shapeDesc) << L"\"" << Endl;
		return 0;
	}

	Ref< Body > body;

	if (const StaticBodyDesc* staticDesc = dynamic_type_cast< const StaticBodyDesc* >(desc))
	{
		physx::PxMaterial* material = m_sdk->createMaterial(staticDesc->getFriction(), staticDesc->getFriction(), 0.1f);
		if (!material)
			return 0;

		if (!staticDesc->isKinematic())
		{
			physx::PxRigidStatic* rigidBody = m_sdk->createRigidStatic(physx::PxTransform(physx::PxIdentity));
			if (!rigidBody)
			{
				log::error << L"Unable to create PhysX rigid body" << Endl;
				return 0;
			}

			physx::PxShape* shape = rigidBody->createShape(*geometry, *material, shapePose);
			T_ASSERT (shape);

			Ref< BodyPhysX > staticBody = new BodyPhysX(
				this,
				m_scene,
				rigidBody,
				centerOfGravity,
				shapeDesc->getCollisionGroup(),
				shapeDesc->getCollisionMask(),
				shapeDesc->getMaterial(),
				tag
			);

			// Setup filter data; pack pointer to our body wrapper into last two words.
			physx::PxFilterData filterData;
			filterData.word0 = shapeDesc->getCollisionGroup();
			filterData.word1 = shapeDesc->getCollisionMask();
			filterData.word2 = reinterpret_cast< physx::PxU32 >(staticBody.ptr());
			filterData.word3 = 0;

			shape->setSimulationFilterData(filterData);
			shape->setQueryFilterData(filterData);

			if (margin > FUZZY_EPSILON)
				shape->setContactOffset(margin);

			m_bodies.push_back(staticBody);

			shape->userData = staticBody;
			body = staticBody;
		}
		else
		{
			physx::PxRigidDynamic* rigidBody = m_sdk->createRigidDynamic(physx::PxTransform(physx::PxIdentity));
			if (!rigidBody)
			{
				log::error << L"Unable to create PhysX actor" << Endl;
				return 0;
			}

			rigidBody->setRigidDynamicFlag(physx::PxRigidDynamicFlag::eKINEMATIC, true);

			physx::PxShape* shape = rigidBody->createShape(*geometry, *material, shapePose);
			T_ASSERT (shape);

			Ref< BodyPhysX > kineticBody = new BodyPhysX(
				this,
				m_scene,
				rigidBody,
				centerOfGravity,
				shapeDesc->getCollisionGroup(),
				shapeDesc->getCollisionMask(),
				shapeDesc->getMaterial(),
				tag
			);

			// Setup filter data; pack pointer to our body wrapper into last two words.
			physx::PxFilterData filterData;
			filterData.word0 = shapeDesc->getCollisionGroup();
			filterData.word1 = shapeDesc->getCollisionMask();
			filterData.word2 = reinterpret_cast< physx::PxU32 >(kineticBody.ptr());
			filterData.word3 = 0;

			shape->setSimulationFilterData(filterData);
			shape->setQueryFilterData(filterData);

			if (margin > FUZZY_EPSILON)
				shape->setContactOffset(margin);

			m_bodies.push_back(kineticBody);

			shape->userData = kineticBody;
			body = kineticBody;
		}
	}
	else if (const DynamicBodyDesc* dynamicDesc = dynamic_type_cast< const DynamicBodyDesc* >(desc))
	{
		physx::PxRigidDynamic* rigidBody = m_sdk->createRigidDynamic(physx::PxTransform(physx::PxIdentity));
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

		physx::PxShape* shape = rigidBody->createShape(*geometry, *material, shapePose);
		T_ASSERT (shape);

		Ref< BodyPhysX > dynamicBody = new BodyPhysX(
			this,
			m_scene,
			rigidBody,
			centerOfGravity,
			shapeDesc->getCollisionGroup(),
			shapeDesc->getCollisionMask(),
			shapeDesc->getMaterial(),
			tag
		);

		// Setup filter data; pack pointer to our body wrapper into last two words.
		physx::PxFilterData filterData;
		filterData.word0 = shapeDesc->getCollisionGroup();
		filterData.word1 = shapeDesc->getCollisionMask();
		filterData.word2 = reinterpret_cast< physx::PxU32 >(dynamicBody.ptr());
		filterData.word3 = 0;

		shape->setSimulationFilterData(filterData);
		shape->setQueryFilterData(filterData);

		if (margin > FUZZY_EPSILON)
			shape->setContactOffset(margin);

		physx::PxRigidBodyExt::setMassAndUpdateInertia(*rigidBody, dynamicDesc->getMass());

		if (!dynamicDesc->getActive())
			rigidBody->putToSleep();

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
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

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
	else if (const FixedJointDesc* fixedDesc = dynamic_type_cast< const FixedJointDesc* >(desc))
	{
		Transform Tbody1Inv = body1->getCenterTransform().inverse();
		Transform Tbody2Inv = body2->getCenterTransform().inverse();

		physx::PxFixedJoint* fixedJoint = physx::PxFixedJointCreate(
			*m_sdk,
			actor1,
			toPxTransform(transform * Tbody1Inv),
			actor2,
			toPxTransform(transform * Tbody2Inv)
		);

		outJoint = new FixedJointPhysX(this, fixedJoint, body1, body2);
	}
	else if (const HingeJointDesc* hingeDesc = dynamic_type_cast< const HingeJointDesc* >(desc))
	{
		Vector4 anchorW = transform * hingeDesc->getAnchor().xyz1();
		Vector4 axisW = transform * hingeDesc->getAxis().xyz0();

		Transform Tbody1Inv = body1->getCenterTransform().inverse();
		Transform Tbody2Inv = body2->getCenterTransform().inverse();

		Vector4 axisIn1 = Tbody1Inv * axisW;
		Vector4 axisIn2 = Tbody2Inv * axisW;

		// Calculate reference axises in body 1 space.
		Vector4 axisA1 = Tbody1Inv.axisX();
		Vector4 axisA2;

		Scalar projection = dot3(axisIn1, axisA1);
		if (projection >= 1.0f - FUZZY_EPSILON)
		{
			axisA1 = -Tbody1Inv.axisZ();
			axisA2 = Tbody1Inv.axisY();
		}
		else if (projection <= -1.0f + FUZZY_EPSILON)
		{
			axisA1 = Tbody1Inv.axisZ();
			axisA2 = Tbody1Inv.axisY();
		}
		else
		{
			axisA2 = cross(axisIn1, axisA1).normalized();
			axisA1 = cross(axisA2, axisIn1).normalized();
		}

		// Calculate reference axises in body 2 space.
		Vector4 axisB1 = Quaternion(axisIn1, axisIn2) * axisA1;
		Vector4 axisB2 = cross(axisIn2, axisB1).normalized();

		Matrix44 R1(
			axisIn1,
			axisA1,
			axisA2,
			Tbody1Inv * anchorW
		);

		Matrix44 R2(
			axisIn2,
			axisB1,
			axisB2,
			Tbody2Inv * anchorW
		);

		physx::PxRevoluteJoint* revoluteJoint = physx::PxRevoluteJointCreate(
			*m_sdk,
			actor1,
			toPxTransform(Transform(R1)),
			actor2,
			toPxTransform(Transform(R2))
		);

		if (hingeDesc->getEnableLimits())
		{
			float minAngle, maxAngle;
			hingeDesc->getAngles(minAngle, maxAngle);

			revoluteJoint->setRevoluteJointFlag(physx::PxRevoluteJointFlag::eLIMIT_ENABLED, true);

			physx::PxJointAngularLimitPair limits(minAngle, maxAngle);
			limits.damping = 0.1f;
			revoluteJoint->setLimit(limits);
		}

		revoluteJoint->setConstraintFlags(physx::PxConstraintFlag::ePROJECTION);

		outJoint = new HingeJointPhysX(this, revoluteJoint, body1, body2);
	}
	else if (const Hinge2JointDesc* hinge2Desc = dynamic_type_cast< const Hinge2JointDesc* >(desc))
	{
		Vector4 anchorW = transform * hinge2Desc->getAnchor().xyz1();
		Vector4 axis1W = transform * hinge2Desc->getAxis1().xyz0();
		Vector4 axis2W = transform * hinge2Desc->getAxis1().xyz0();

		Transform Tbody1Inv = body1->getCenterTransform().inverse();
		Transform Tbody2Inv = body2->getCenterTransform().inverse();

		Vector4 axis1In1 = Tbody1Inv * axis1W;
		Vector4 axis1In2 = Tbody2Inv * axis1W;

		// Calculate reference axises in body 1 space.
		Vector4 axisA1 = Tbody1Inv.axisX();
		Vector4 axisA2;

		Scalar projection = dot3(axis1In1, axisA1);
		if (projection >= 1.0f - FUZZY_EPSILON)
		{
			axisA1 = -Tbody1Inv.axisZ();
			axisA2 = Tbody1Inv.axisY();
		}
		else if (projection <= -1.0f + FUZZY_EPSILON)
		{
			axisA1 = Tbody1Inv.axisZ();
			axisA2 = Tbody1Inv.axisY();
		}
		else
		{
			axisA2 = cross(axis1In1, axisA1).normalized();
			axisA1 = cross(axisA2, axis1In1).normalized();
		}

		// Calculate reference axises in body 2 space.
		Vector4 axisB1 = Quaternion(axis1In1, axis1In2) * axisA1;
		Vector4 axisB2 = cross(axis1In2, axisB1).normalized();

		Matrix44 R1(
			axis1In1,
			axisA1,
			axisA2,
			Tbody1Inv * anchorW
		);

		Matrix44 R2(
			axis1In2,
			axisB1,
			axisB2,
			Tbody2Inv * anchorW
		);

		physx::PxRevoluteJoint* revoluteJoint = physx::PxRevoluteJointCreate(
			*m_sdk,
			actor1,
			toPxTransform(Transform(R1)),
			actor2,
			toPxTransform(Transform(R2))
		);

		outJoint = new Hinge2JointPhysX(this, revoluteJoint, body1, body2);
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
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	m_collisionInfo.resize(0);

	m_scene->simulate(m_simulationDeltaTime * m_timeScale);
	if (!m_scene->fetchResults(true))
		log::error << L"Unable to fetch simulation results; physics may be inconsistent" << Endl;

	if (issueCollisionEvents)
	{
		for (AlignedVector< CollisionInfo >::const_iterator i = m_collisionInfo.begin(); i != m_collisionInfo.end(); ++i)
			notifyCollisionListeners(*i);
	}
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
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	physx::PxRaycastHit hit;

	if (ignoreClusterId == 0)
	{
		physx::PxQueryFilterData filterData;
		filterData.data.word0 = group;
		filterData.data.word1 = 0;
		filterData.data.word2 = 0;
		filterData.data.word3 = 0;
		filterData.flags = physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC;

		if (!m_scene->raycastSingle(
			toPxVec3(at),
			toPxVec3(direction),
			maxLength,
			physx::PxHitFlag::ePOSITION | physx::PxHitFlag::eNORMAL | physx::PxHitFlag::eDISTANCE,
			hit,
			filterData
		))
			return false;
	}
	else
	{
		IgnoreBodyFilter filter(ignoreClusterId, group);

		physx::PxQueryFilterData filterData;
		filterData.data.word0 = group;
		filterData.data.word1 = 0;
		filterData.data.word2 = 0;
		filterData.data.word3 = 0;
		filterData.flags = physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::ePREFILTER;

		if (!m_scene->raycastSingle(
			toPxVec3(at),
			toPxVec3(direction),
			maxLength,
			physx::PxHitFlag::ePOSITION | physx::PxHitFlag::eNORMAL | physx::PxHitFlag::eDISTANCE,
			hit,
			filterData,
			&filter
		))
			return false;
	}

	BodyPhysX* body = reinterpret_cast< BodyPhysX* >(hit.shape->userData);
	T_ASSERT (body);
	T_ASSERT ((body->getCollisionGroup() & group) != 0);

	outResult.distance = hit.distance;
	outResult.position = fromPxVec3(hit.position, 1.0f);
	outResult.normal = fromPxVec3(hit.normal, 0.0f);
	outResult.body = body;
	outResult.material = body->getMaterial();

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
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	physx::PxRaycastHit hit;

	if (ignoreClusterId == 0)
	{
		physx::PxQueryFilterData filterData;
		filterData.data.word0 = group;
		filterData.data.word1 = 0;
		filterData.data.word2 = 0;
		filterData.data.word3 = 0;

		if (queryTypes & QtStatic)
			filterData.flags |= physx::PxQueryFlag::eSTATIC;
		if (queryTypes & QtDynamic)
			filterData.flags |= physx::PxQueryFlag::eDYNAMIC;

		if (!m_scene->raycastAny(
			toPxVec3(at),
			toPxVec3(direction),
			maxLength,
			hit,
			filterData
		))
			return false;
	}
	else
	{
		IgnoreBodyFilter filter(ignoreClusterId, group);

		physx::PxQueryFilterData filterData;
		filterData.data.word0 = group;
		filterData.data.word1 = 0;
		filterData.data.word2 = 0;
		filterData.data.word3 = 0;
		filterData.flags = physx::PxQueryFlag::ePREFILTER;

		if (queryTypes & QtStatic)
			filterData.flags |= physx::PxQueryFlag::eSTATIC;
		if (queryTypes & QtDynamic)
			filterData.flags |= physx::PxQueryFlag::eDYNAMIC;

		if (!m_scene->raycastAny(
			toPxVec3(at),
			toPxVec3(direction),
			maxLength,
			hit,
			filterData,
			&filter
		))
			return false;
	}

	return true;
}

uint32_t PhysicsManagerPhysX::querySphere(
	const Vector4& at,
	float radius,
	uint32_t group,
	uint32_t queryTypes,
	RefArray< Body >& outBodies
) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	physx::PxSphereGeometry sphereGeometry(radius);
	physx::PxOverlapHit hits[64];

	physx::PxQueryFilterData filterData;
	filterData.data.word0 = group;
	filterData.data.word1 = 0;
	filterData.data.word2 = 0;
	filterData.data.word3 = 0;

	if (queryTypes & QtStatic)
		filterData.flags |= physx::PxQueryFlag::eSTATIC;
	if (queryTypes & QtDynamic)
		filterData.flags |= physx::PxQueryFlag::eDYNAMIC;

	physx::PxI32 nhits = m_scene->overlapMultiple(
		sphereGeometry,
		physx::PxTransform::PxTransform(toPxVec3(at)),
		hits,
		sizeof_array(hits),
		filterData
	);

	if (nhits <= 0)
		return 0;

	outBodies.reserve(nhits);
	for (physx::PxI32 i = 0; i < nhits; ++i)
	{
		BodyPhysX* body = reinterpret_cast< BodyPhysX* >(hits[i].shape->userData);
		T_ASSERT (body);
		T_ASSERT ((body->getCollisionGroup() & group) != 0);
		outBodies.push_back(body);
	}

	return nhits;
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
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	physx::PxSphereGeometry sphereGeometry(radius);

	physx::PxSweepHit hit;

	if (ignoreClusterId == 0)
	{
		physx::PxQueryFilterData filterData;
		filterData.data.word0 = group;
		filterData.data.word1 = 0;
		filterData.data.word2 = 0;
		filterData.data.word3 = 0;
		filterData.flags = physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC;

		if (!m_scene->sweepSingle(
			sphereGeometry,
			physx::PxTransform::PxTransform(toPxVec3(at)),
			toPxVec3(direction),
			maxLength,
			physx::PxHitFlag::ePOSITION | physx::PxHitFlag::eNORMAL | physx::PxHitFlag::eDISTANCE,
			hit,
			filterData
		))
			return false;
	}
	else
	{
		IgnoreBodyFilter filter(ignoreClusterId, group);

		physx::PxQueryFilterData filterData;
		filterData.data.word0 = group;
		filterData.data.word1 = 0;
		filterData.data.word2 = 0;
		filterData.data.word3 = 0;
		filterData.flags = physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::ePREFILTER;

		if (!m_scene->sweepSingle(
			sphereGeometry,
			physx::PxTransform::PxTransform(toPxVec3(at)),
			toPxVec3(direction),
			maxLength,
			physx::PxHitFlag::ePOSITION | physx::PxHitFlag::eNORMAL | physx::PxHitFlag::eDISTANCE,
			hit,
			filterData,
			&filter
		))
			return false;
	}

	BodyPhysX* body = reinterpret_cast< BodyPhysX* >(hit.shape->userData);
	T_ASSERT (body);
	T_ASSERT ((body->getCollisionGroup() & group) != 0);

	outResult.distance = hit.distance + radius;
	outResult.position = fromPxVec3(hit.position, 1.0f);
	outResult.normal = fromPxVec3(hit.normal, 0.0f);
	outResult.body = body;
	outResult.material = body->getMaterial();

	return true;
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
	AlignedVector< QueryResult >& outResult
) const
{
}

void PhysicsManagerPhysX::queryOverlap(
	const Body* body,
	RefArray< Body >& outResult
) const
{
}

void PhysicsManagerPhysX::queryTriangles(
	const Vector4& center,
	float radius,
	AlignedVector< TriangleResult >& outTriangles
) const
{
}

void PhysicsManagerPhysX::getStatistics(PhysicsStatistics& outStatistics) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	outStatistics.bodyCount = m_bodies.size();
	outStatistics.activeCount = 0;
	outStatistics.manifoldCount = 0;
	outStatistics.queryCount = 0;

	for (RefArray< BodyPhysX >::const_iterator i = m_bodies.begin(); i != m_bodies.end(); ++i)
	{
		if ((*i)->isActive())
			outStatistics.activeCount++;
	}
}

void PhysicsManagerPhysX::insertActor(physx::PxRigidActor* actor)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_scene->addActor(*actor);
}

void PhysicsManagerPhysX::removeActor(physx::PxRigidActor* actor)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_scene->removeActor(*actor);
}

void PhysicsManagerPhysX::destroyBody(Body* owner, physx::PxRigidActor* actor)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	RefArray< BodyPhysX >::iterator i = std::find(m_bodies.begin(), m_bodies.end(), owner);
	if (i != m_bodies.end())
		m_bodies.erase(i);

	actor->release();
}

void PhysicsManagerPhysX::destroyJoint(Joint* owner, physx::PxJoint* joint)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	joint->release();
}

	}
}
