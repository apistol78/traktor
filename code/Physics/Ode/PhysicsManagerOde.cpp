#include <limits>
#include <algorithm>
#include <ode/ode.h>
#include "Physics/Ode/PhysicsManagerOde.h"
#include "Physics/Ode/StaticBodyOde.h"
#include "Physics/Ode/DynamicBodyOde.h"
#include "Physics/Ode/BallJointOde.h"
#include "Physics/Ode/ConeTwistJointOde.h"
#include "Physics/Ode/HingeJointOde.h"
#include "Physics/Ode/Hinge2JointOde.h"
#include "Physics/Ode/Types.h"
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
#include "Core/Math/Const.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace physics
	{
		namespace
		{

struct RayCallbackData
{
	dGeomID ignoreGeomId;
	dContactGeom contact;
};

void nearRayCallback(void* data, dGeomID o1, dGeomID o2)
{
	RayCallbackData& rayData = *reinterpret_cast< RayCallbackData* >(data);

	if (rayData.ignoreGeomId == o1 || rayData.ignoreGeomId == o2)
		return;

	dContactGeom contact;
	if (dCollide(o1, o2, 1, &contact, sizeof(dContactGeom)))
	{
		if (contact.depth < rayData.contact.depth)
			rayData.contact = contact;
	}
}

		}

class PhysicsManagerOdeImpl : public DestroyCallback
{
public:
	PhysicsManagerOdeImpl(PhysicsManagerOde* physicsManager)
	:	m_physicsManager(physicsManager)
	,	m_worldId(0)
	,	m_jointGroupId(0)
	,	m_contactGroupId(0)
	,	m_spaceId(0)
	{
	}

	bool create(const PhysicsManagerOde::Settings& settings)
	{
		m_settings = settings;

		m_worldId = dWorldCreate();
		m_spaceId = dHashSpaceCreate(0);
		m_jointGroupId = dJointGroupCreate(0);
		m_contactGroupId = dJointGroupCreate(0);

		dWorldSetGravity(m_worldId, 0.0f, 0.0f, 0.0f);
		dWorldSetERP(m_worldId, m_settings.globalErp);
		dWorldSetCFM(m_worldId, m_settings.globalCfm);
		dWorldSetQuickStepNumIterations(m_worldId, m_settings.fastStepIterations);

		return true;
	}

	void destroy()
	{
		while (!m_joints.empty())
			m_joints.front()->destroy();
		while (!m_dynamicBodies.empty())
			m_dynamicBodies.front()->destroy();
		while (!m_staticBodies.empty())
			m_staticBodies.front()->destroy();

		if (m_contactGroupId)
		{
			dJointGroupDestroy(m_contactGroupId);
			m_contactGroupId = 0;
		}
		if (m_jointGroupId)
		{
			dJointGroupDestroy(m_jointGroupId);
			m_jointGroupId = 0;
		}
		if (m_spaceId)
		{
			dSpaceDestroy(m_spaceId);
			m_spaceId = 0;
		}
		if (m_worldId)
		{
			dWorldDestroy(m_worldId);
			m_worldId = 0;
		}
	}

	void setGravity(const Vector4& gravity)
	{
		dWorldSetGravity(m_worldId, gravity.x(), gravity.y(), gravity.z());
	}

	Body* createBody(const BodyDesc* desc)
	{
		if (!desc)
			return 0;

		const ShapeDesc* shapeDesc = desc->getShape();
		if (!shapeDesc)
		{
			log::error << L"Unable to create body, no shape defined" << Endl;
			return 0;
		}

		// Create collision shape.
		dTriMeshDataID meshDataId = 0;
		dGeomID geomId = 0;
		dSpaceID spaceId = 0;

		const Matrix44& localTransform = shapeDesc->getLocalTransform();
		if (localTransform == Matrix44::identity())
			spaceId = m_spaceId;

		if (const BoxShapeDesc* boxShape = dynamic_type_cast< const BoxShapeDesc* >(shapeDesc))
		{
			geomId = dCreateBox(
				spaceId,
				boxShape->getExtent().x() * 2.0f,
				boxShape->getExtent().y() * 2.0f,
				boxShape->getExtent().z() * 2.0f
			);
		}
		else if (const CapsuleShapeDesc* capsuleShape = dynamic_type_cast< const CapsuleShapeDesc* >(shapeDesc))
		{
			geomId = dCreateCapsule(
				spaceId,
				capsuleShape->getRadius(),
				capsuleShape->getLength() - capsuleShape->getRadius() * 2.0f
			);
		}
		else if (const CylinderShapeDesc* cylinderShape = dynamic_type_cast< const CylinderShapeDesc* >(shapeDesc))
		{
			geomId = dCreateCCylinder(
				spaceId,
				cylinderShape->getRadius(),
				cylinderShape->getLength()
			);
		}
		else if (const MeshShapeDesc* meshShape = dynamic_type_cast< const MeshShapeDesc* >(shapeDesc))
		{
			resource::Proxy< Mesh > mesh = meshShape->getMesh();
			if (!mesh.validate())
			{
				log::error << L"Unable to create body, unable to read mesh" << Endl;
				return 0;
			}

			meshDataId = dGeomTriMeshDataCreate();
			T_ASSERT (meshDataId);

			dGeomTriMeshDataBuildSingle(
				meshDataId,
				(const void *)&mesh->getVertices().front(),
				sizeof(float) * 3,
				(int)mesh->getVertices().size(),
				(const void *)&mesh->getShapeTriangles().front(),
				(int)mesh->getShapeTriangles().size() * 3,
				sizeof(uint32_t) * 3
			);

			geomId = dCreateTriMesh(
				spaceId,
				meshDataId,
				0,
				0,
				0
			);
		}
		else if (const SphereShapeDesc* sphereShape = dynamic_type_cast< const SphereShapeDesc* >(shapeDesc))
		{
			geomId = dCreateSphere(
				spaceId,
				sphereShape->getRadius()
			);
		}
		else if (const HeightfieldShapeDesc* heightfieldShape = dynamic_type_cast< const HeightfieldShapeDesc* >(shapeDesc))
		{
			resource::Proxy< Heightfield > heightfield = heightfieldShape->getHeightfield();
			if (!heightfield.validate())
			{
				log::error << L"Unable to load heightfield resource" << Endl;
				return 0;
			}

			dHeightfieldDataID heightId = dGeomHeightfieldDataCreate();

			dGeomHeightfieldDataBuildCallback(
				heightId,
				heightfield.getPtr(),
				&PhysicsManagerOdeImpl::heightfieldCallback,
				heightfield->getWorldExtent().x(),
				heightfield->getWorldExtent().z(),
				heightfield->getSize(),
				heightfield->getSize(),
				1.0f,
				0.0f,
				0.0f,
				0
			);

			dGeomHeightfieldDataSetBounds(
				heightId,
				-heightfield->getWorldExtent().y(),
				heightfield->getWorldExtent().y()
			);

			geomId = dCreateHeightfield(spaceId, heightId, 1);
		}

		if (!geomId)
		{
			log::error << L"Unable to create body, unknown shape type \"" << type_name(shapeDesc) << L"\"" << Endl;
			return 0;
		}

		// Adjust shape transformation relative to body.
		if (spaceId == 0)
		{
			dGeomID transformGeomId = dCreateGeomTransform(m_spaceId);
			dGeomTransformSetGeom(transformGeomId, geomId);
			dGeomTransformSetCleanup(transformGeomId, 1);

			const Vector4& p = localTransform.translation();
			dGeomSetPosition(geomId, p.x(), p.y(), p.z());

			const float* e = localTransform.m;
			dMatrix3 rotation =
			{
				e[0], e[4], e[8], 0.0f,
				e[1], e[5], e[9], 0.0f,
				e[2], e[6], e[10], 0.0f
			};
			dGeomSetRotation(geomId, rotation);

			geomId = transformGeomId;
		}

		// Create body.
		Ref< Body > body;

		if (const StaticBodyDesc* staticDesc = dynamic_type_cast< const StaticBodyDesc* >(desc))
		{
			Ref< StaticBodyOde > staticBody = gc_new< StaticBodyOde >(this, geomId);
			m_staticBodies.push_back(staticBody);
			body = staticBody;
		}
		else if (const DynamicBodyDesc* dynamicDesc = dynamic_type_cast< const DynamicBodyDesc* >(desc))
		{
			dBodyID bodyId = dBodyCreate(m_worldId);
			T_ASSERT (bodyId);

			dGeomSetBody(geomId, bodyId);

			dMass mass;

			if (const BoxShapeDesc* boxShape = dynamic_type_cast< const BoxShapeDesc* >(shapeDesc))
			{
				dMassSetBox(
					&mass,
					1.0f,
					boxShape->getExtent().x(),
					boxShape->getExtent().y(),
					boxShape->getExtent().z()
				);
			}
			else if (const CapsuleShapeDesc* capsuleShape = dynamic_type_cast< const CapsuleShapeDesc* >(shapeDesc))
			{
				dMassSetCapsule(
					&mass,
					1.0f,
					3,
					capsuleShape->getRadius(),
					capsuleShape->getLength()
				);
			}
			else if (const CylinderShapeDesc* cylinderShape = dynamic_type_cast< const CylinderShapeDesc* >(shapeDesc))
			{
				dMassSetCylinder(
					&mass,
					1.0f,
					3,
					cylinderShape->getRadius(),
					cylinderShape->getLength()
				);
			}
			else if (const MeshShapeDesc* meshShape = dynamic_type_cast< const MeshShapeDesc* >(shapeDesc))
			{
				dMassSetBox(
					&mass,
					1.0f,
					1.0f,
					1.0f,
					1.0f
				);
			}
			else if (const SphereShapeDesc* sphereShape = dynamic_type_cast< const SphereShapeDesc* >(shapeDesc))
			{
				dMassSetSphere(
					&mass,
					1.0f,
					sphereShape->getRadius()
				);
			}

			dMassAdjust(
				&mass,
				dynamicDesc->getMass()
			);

			//dMassRotate(&mass, rotation);
			//dMassTranslate(&mass, p.x, p.y, p.z);

			dBodySetMass(bodyId, &mass);

			if (dynamicDesc->getAutoDisable())
				dBodySetAutoDisableFlag(bodyId, true);

			Ref< DynamicBodyOde > dynamicBody = gc_new< DynamicBodyOde >(
				this,
				m_worldId,
				m_contactGroupId,
				meshDataId,
				geomId,
				bodyId
			);
			m_dynamicBodies.push_back(dynamicBody);
			body = dynamicBody;
		}
		else
		{
			log::error << L"Unsupported body type \"" << type_name(desc) << L"\"" << Endl;
			return 0;
		}

		return body;
	}

	Joint* createJoint(const JointDesc* desc, const Matrix44& transform, Body* body1, Body* body2)
	{
		if (!desc)
			return 0;

		Ref< Joint > joint;

		if (const BallJointDesc* ballDesc = dynamic_type_cast< const BallJointDesc* >(desc))
		{
			dJointID jointId = dJointCreateBall(m_worldId, m_jointGroupId);
			T_ASSERT (jointId);

			dBodyID b1 = 0, b2 = 0;

			if (DynamicBodyOde* dynamicBody1 = dynamic_type_cast< DynamicBodyOde* >(body1))
				b1 = dynamicBody1->getBodyId();

			if (DynamicBodyOde* dynamicBody2 = dynamic_type_cast< DynamicBodyOde* >(body2))
				b2 = dynamicBody2->getBodyId();

			dJointAttach(jointId, b1, b2);

			Vector4 anchor = transform * ballDesc->getAnchor().xyz1();
			dJointSetBallAnchor(jointId, anchor.x(), anchor.y(), anchor.z());

			joint = gc_new< BallJointOde >(this, jointId, body1, body2);
		}
		else if (const ConeTwistJointDesc* coneTwistDesc = dynamic_type_cast< const ConeTwistJointDesc* >(desc))
		{
			joint = gc_new< ConeTwistJointOde >(
				this,
				body1,
				body2,
				coneTwistDesc
			);
		}
		else if (const HingeJointDesc* hingeDesc = dynamic_type_cast< const HingeJointDesc* >(desc))
		{
			dJointID jointId = dJointCreateHinge(m_worldId, m_jointGroupId);
			T_ASSERT (jointId);

			dBodyID b1 = 0, b2 = 0;

			if (DynamicBodyOde* dynamicBody1 = dynamic_type_cast< DynamicBodyOde* >(body1))
				b1 = dynamicBody1->getBodyId();

			if (DynamicBodyOde* dynamicBody2 = dynamic_type_cast< DynamicBodyOde* >(body2))
				b2 = dynamicBody2->getBodyId();

			dJointAttach(jointId, b1, b2);

			Vector4 anchor = transform * hingeDesc->getAnchor().xyz1();
			Vector4 axis = transform * hingeDesc->getAxis().xyz0();

			dJointSetHingeAnchor(jointId, anchor.x(), anchor.y(), anchor.z());
			dJointSetHingeAxis(jointId, axis.x(), axis.y(), axis.z());

			float minAngle, maxAngle;
			hingeDesc->getAngles(minAngle, maxAngle);
			if (abs(maxAngle - minAngle) >= FUZZY_EPSILON)
			{
				dJointSetHingeParam(jointId, dParamLoStop, minAngle);
				dJointSetHingeParam(jointId, dParamHiStop, maxAngle);
			}

			joint = gc_new< HingeJointOde >(this, jointId, body1, body2);
		}
		else if (const Hinge2JointDesc* hingeDesc = dynamic_type_cast< const Hinge2JointDesc* >(desc))
		{
			dJointID jointId = dJointCreateHinge2(m_worldId, m_jointGroupId);
			T_ASSERT (jointId);

			dBodyID b1 = 0, b2 = 0;

			if (DynamicBodyOde* dynamicBody1 = dynamic_type_cast< DynamicBodyOde* >(body1))
				b1 = dynamicBody1->getBodyId();

			if (DynamicBodyOde* dynamicBody2 = dynamic_type_cast< DynamicBodyOde* >(body2))
				b2 = dynamicBody2->getBodyId();

			dJointAttach(jointId, b1, b2);

			Vector4 anchor = transform * hingeDesc->getAnchor().xyz1();
			Vector4 axis1 = transform * hingeDesc->getAxis1().xyz0();
			Vector4 axis2 = transform * hingeDesc->getAxis2().xyz0();

			dJointSetHinge2Anchor(jointId, anchor.x(), anchor.y(), anchor.z());
			dJointSetHinge2Axis1(jointId, axis1.x(), axis1.y(), axis1.z());
			dJointSetHinge2Axis2(jointId, axis2.x(), axis2.y(), axis2.z());
			dJointSetHinge2Param(jointId, dParamLoStop, hingeDesc->getLowStop());
			dJointSetHinge2Param(jointId, dParamHiStop, hingeDesc->getHighStop());
			dJointSetHinge2Param(jointId, dParamSuspensionERP, hingeDesc->getSuspensionErp());
			dJointSetHinge2Param(jointId, dParamSuspensionCFM, hingeDesc->getSuspensionCfm());

			joint = gc_new< Hinge2JointOde >(this, jointId, body1, body2);
		}

		if (!joint)
		{
			log::error << L"Unable to create joint, unknown joint type \"" << type_name(desc) << L"\"" << Endl;
			return 0;
		}

		m_joints.push_back(joint);
		return joint;
	}

	void update()
	{
		// Save current state on dynamic bodies.
		for (RefArray< DynamicBodyOde >::iterator i = m_dynamicBodies.begin(); i != m_dynamicBodies.end(); ++i)
			(*i)->setPreviousState((*i)->getState());

		// Check for collisions.
		dSpaceCollide(m_spaceId, reinterpret_cast< void* >(this), nearCallbackStatic);

		//// Update specially implemented joints.
		//for (RefArray< Joint >::iterator i = m_joints.begin(); i != m_joints.end(); ++i)
		//	(*i)->update(deltaTime);

		// Step simulation.
		float stepSize = m_settings.simulationDeltaTime;
		if (m_settings.fastStepEnable)
			dWorldQuickStep(m_worldId, stepSize);
		else
			dWorldStep(m_worldId, stepSize);

		// Clear collision contacts.
		dJointGroupEmpty(m_contactGroupId);
	}

	bool queryRay(const Vector4& at, const Vector4& direction, float maxLength, const Body* ignoreBody, QueryResult& outResult) const
	{
		RayCallbackData rayData = { 0 };

		if (ignoreBody)
		{
			if (is_a< StaticBodyOde >(ignoreBody))
				rayData.ignoreGeomId = static_cast< const StaticBodyOde* >(ignoreBody)->getGeomId();
			if (is_a< DynamicBodyOde >(ignoreBody))
				rayData.ignoreGeomId = static_cast< const DynamicBodyOde* >(ignoreBody)->getGeomId();

			T_ASSERT (rayData.ignoreGeomId);
		}

		dGeomID ray = dCreateRay(m_spaceId, maxLength);
		dGeomRaySet(ray, at.x(), at.y(), at.z(), direction.x(), direction.y(), direction.z());

		rayData.contact.depth = std::numeric_limits< float >::max();
		dSpaceCollide2((dGeomID)m_spaceId, ray, &rayData, &nearRayCallback);

		dGeomDestroy(ray);

		if (rayData.contact.depth <= maxLength)
		{
			T_ASSERT (rayData.contact.g1 != rayData.ignoreGeomId);
			T_ASSERT (rayData.contact.g2 != rayData.ignoreGeomId);

			outResult.position = Vector4(rayData.contact.pos[0], rayData.contact.pos[1], rayData.contact.pos[2], 1.0f);
			outResult.normal = Vector4(-rayData.contact.normal[0], -rayData.contact.normal[1], -rayData.contact.normal[2], 0.0f);
			outResult.distance = rayData.contact.depth;

			return true;
		}

		return false;
	}

	virtual void bodyDestroyed(Body* body)
	{
		if (StaticBodyOde* staticBody = dynamic_type_cast< StaticBodyOde* >(body))
		{
			RefArray< StaticBodyOde >::iterator i = std::find(m_staticBodies.begin(), m_staticBodies.end(), staticBody);
			if (i != m_staticBodies.end())
				m_staticBodies.erase(i);
		}
		else if (DynamicBodyOde* dynamicBody = dynamic_type_cast< DynamicBodyOde* >(body))
		{
			RefArray< DynamicBodyOde >::iterator i = std::find(m_dynamicBodies.begin(), m_dynamicBodies.end(), dynamicBody);
			if (i != m_dynamicBodies.end())
				m_dynamicBodies.erase(i);
		}
	}

	virtual void jointDestroyed(Joint* joint)
	{
		RefArray< Joint >::iterator i = std::find(m_joints.begin(), m_joints.end(), joint);
		if (i != m_joints.end())
			m_joints.erase(i);
	}

private:
	PhysicsManagerOde* m_physicsManager;
	PhysicsManagerOde::Settings m_settings;
	dWorldID m_worldId;
	dJointGroupID m_jointGroupId;
	dJointGroupID m_contactGroupId;
	dSpaceID m_spaceId;
	RefArray< StaticBodyOde > m_staticBodies;
	RefArray< DynamicBodyOde > m_dynamicBodies;
	RefArray< Joint > m_joints;

	void nearCallback(dGeomID o1, dGeomID o2)
	{
		dBodyID b1 = dGeomGetBody(o1);
		dBodyID b2 = dGeomGetBody(o2);

		// Bodies connected through a joint should not generate collisions.
		if(b1 && b2 && dAreConnectedExcluding(b1, b2, dJointTypeContact))
			return;

		// Bodies connected through a joint should not generate collisions,
		// this time take special joints into account.
		Body* body1 = reinterpret_cast< Body* >(dGeomGetData(o1)); 
		Body* body2 = reinterpret_cast< Body* >(dGeomGetData(o2));

		for (RefArray< Joint >::iterator i = m_joints.begin(); i != m_joints.end(); ++i)
		{
			if (
				((*i)->getBody1() == body1 && (*i)->getBody2() == body2) ||
				((*i)->getBody1() == body2 && (*i)->getBody2() == body1)
			)
				return;
		}

		// Initialize contact structures.
		std::vector< dContact > contact(m_settings.maxContacts);
		for(int i = 0; i < m_settings.maxContacts; ++i)
		{
			contact[i].surface.mode = dContactSoftERP | dContactSoftCFM | dContactApprox1;
			contact[i].surface.mu = 0.8f;
			contact[i].surface.soft_erp = dReal(0.5);
			contact[i].surface.soft_cfm = dReal(0.3);
		}

		int ncontacts = dCollide(o1, o2, m_settings.maxContacts, &contact[0].geom, sizeof(dContact));
		if(ncontacts > 0)
		{
			// Issue collision listeners.
			CollisionInfo collisionInfo;

			collisionInfo.body1 = body1;
			collisionInfo.body2 = body2;
			collisionInfo.contacts.resize(ncontacts);
			for(int i = 0; i < ncontacts; ++i)
			{
				collisionInfo.contacts[i].position = Vector4(contact[i].geom.pos[0], contact[i].geom.pos[1], contact[i].geom.pos[2], 1.0f);
				collisionInfo.contacts[i].normal = Vector4(contact[i].geom.normal[0], contact[i].geom.normal[1], contact[i].geom.normal[2], 0.0f);
				collisionInfo.contacts[i].depth = contact[i].geom.depth;
			}

			m_physicsManager->notifyCollisionListeners(collisionInfo);

			// Create contact joints.
			for(int i = 0; i < ncontacts; ++i)
			{
				dJointID c = dJointCreateContact(m_worldId, m_contactGroupId, &contact[i]);
				dJointAttach(c, b1, b2);
			}
		}
	}

	static void nearCallbackStatic(void* data, dGeomID o1, dGeomID o2)
	{
		reinterpret_cast< PhysicsManagerOdeImpl* >(data)->nearCallback(o1, o2);
	}

	static dReal heightfieldCallback(void* user, int x, int z)
	{
		Heightfield* heightfield = reinterpret_cast< Heightfield* >(user);
		T_ASSERT (heightfield);

		const float* heights = heightfield->getHeights();
		return dReal(heights[x + z * heightfield->getSize()]);
	}
};

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.physics.PhysicsManagerOde", PhysicsManagerOde, PhysicsManager)

PhysicsManagerOde::PhysicsManagerOde()
:	m_impl(0)
{
	m_impl = new PhysicsManagerOdeImpl(this);
}

PhysicsManagerOde::~PhysicsManagerOde()
{
	delete m_impl;
}

bool PhysicsManagerOde::create(const Settings& settings)
{
	return m_impl->create(settings);
}

bool PhysicsManagerOde::create(float simulationDeltaTime)
{
	Settings settings;
	settings.simulationDeltaTime = simulationDeltaTime;
	return create(settings);
}

void PhysicsManagerOde::destroy()
{
	m_impl->destroy();
}

void PhysicsManagerOde::setGravity(const Vector4& gravity)
{
	m_impl->setGravity(gravity);
}

Body* PhysicsManagerOde::createBody(const BodyDesc* desc)
{
	return m_impl->createBody(desc);
}

Joint* PhysicsManagerOde::createJoint(const JointDesc* desc, const Matrix44& transform, Body* body1, Body* body2)
{
	return m_impl->createJoint(desc, transform, body1, body2);
}

void PhysicsManagerOde::update()
{
	m_impl->update();
}

uint32_t PhysicsManagerOde::getCollidingPairs(std::vector< CollisionPair >& outCollidingPairs) const
{
	return 0;
}

bool PhysicsManagerOde::queryPoint(const Vector4& at, float margin, QueryResult& outResult) const
{
	return false;
}

bool PhysicsManagerOde::queryRay(const Vector4& at, const Vector4& direction, float maxLength, const Body* ignoreBody, QueryResult& outResult) const
{
	return m_impl->queryRay(at, direction, maxLength, ignoreBody, outResult);
}

uint32_t PhysicsManagerOde::querySphere(const Vector4& at, float radius, uint32_t queryTypes, RefArray< Body >& outBodies) const
{
	return 0;
}

bool PhysicsManagerOde::querySweep(const Vector4& at, const Vector4& direction, float maxLength, float radius, const Body* ignoreBody, QueryResult& outResult) const
{
	return false;
}

void PhysicsManagerOde::getBodyCount(uint32_t& outCount, uint32_t& outActiveCount) const
{
}

	}
}
