#include "Core/Math/Const.h"
//#include "Physics/BallJointDesc.h"
//#include "Physics/ConeTwistJointDesc.h"
//#include "Physics/HingeJointDesc.h"
//#include "Physics/Hinge2JointDesc.h"
#include "Physics/Editor/ArticulatedEntityEditor.h"
#include "Physics/World/ArticulatedEntity.h"
#include "Physics/World/ArticulatedEntityData.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"
#include "World/Entity/SpatialEntity.h"
#include "World/Entity/SpatialEntityData.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.ArticulatedEntityEditor", ArticulatedEntityEditor, scene::DefaultEntityEditor)

ArticulatedEntityEditor::ArticulatedEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter)
:	scene::DefaultEntityEditor(context, entityAdapter)
{
}

void ArticulatedEntityEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	ArticulatedEntityData* articulatedEntityData = checked_type_cast< ArticulatedEntityData* >(getEntityAdapter()->getEntityData());

	const std::vector< ArticulatedEntityData::Constraint >& constraints = articulatedEntityData->getConstraints();
	Transform transform = articulatedEntityData->getTransform();

	const RefArray< scene::EntityAdapter >& constraintChildren = getEntityAdapter()->getChildren();

	for (uint32_t i = 0; i < uint32_t(constraints.size()); ++i)
	{
		const ArticulatedEntityData::Constraint& constraint = constraints[i];
		if (constraint.entityIndex1 < 0)
			continue;

		scene::EntityAdapter* entity1 = constraintChildren[constraint.entityIndex1];
		if (!entity1)
			continue;

		scene::EntityAdapter* entity2 = (constraint.entityIndex2 >= 0) ? constraintChildren[constraint.entityIndex2] : 0;
		if (entity2)
		{
			Transform body1Transform0 = entity1->getTransform0();
			Transform body1Transform = entity1->getTransform();
			Transform body2Transform0 = entity2->getTransform0();
			Transform body2Transform = entity2->getTransform();

			m_physicsRenderer.draw(
				primitiveRenderer,
				body1Transform0,
				body1Transform,
				body2Transform0,
				body2Transform,
				constraint.jointDesc
			);
		}
		else
		{
			Transform body1Transform0 = entity1->getTransform0();
			Transform body1Transform = entity1->getTransform();

			m_physicsRenderer.draw(
				primitiveRenderer,
				body1Transform0,
				body1Transform,
				constraint.jointDesc
			);
		}

		/*
		Transform body1OriginalTransformInv = entity1->getTransform().inverse();
		Transform body1Transform = entity1->getTransform();
		Transform jointTransform = transform * body1OriginalTransformInv * body1Transform;
		Vector4 body1Center = body1Transform.translation();

		if (const BallJointDesc* ballJointDesc = dynamic_type_cast< const BallJointDesc* >(constraint.jointDesc))
		{
			Vector4 anchor = jointTransform * ballJointDesc->getAnchor().xyz1();

			primitiveRenderer->drawSolidPoint(anchor, 4.0f, Color4ub(255, 255, 255));

			primitiveRenderer->drawLine(
				body1Center,
				anchor,
				Color4ub(255, 255, 255)
			);

			if (entity2)
			{
				Vector4 body2Center = entity2->getTransform().translation();
				primitiveRenderer->drawLine(
					body2Center,
					anchor,
					Color4ub(255, 255, 255)
				);
			}
		}
		else if (const ConeTwistJointDesc* coneTwistJointDesc = dynamic_type_cast< const ConeTwistJointDesc* >(constraint.jointDesc))
		{
			const Scalar c_axisLength(5.0f);
			const float c_limitLength = 2.0f;

			Vector4 anchor = jointTransform * coneTwistJointDesc->getAnchor().xyz1();

			primitiveRenderer->drawSolidPoint(anchor, 4.0f, Color4ub(255, 255, 255));

			primitiveRenderer->drawLine(
				anchor - coneTwistJointDesc->getTwistAxis() * c_axisLength,
				anchor + coneTwistJointDesc->getTwistAxis() * c_axisLength,
				Color4ub(255, 255, 0)
			);
			primitiveRenderer->drawLine(
				anchor - coneTwistJointDesc->getConeAxis() * c_axisLength,
				anchor + coneTwistJointDesc->getConeAxis() * c_axisLength,
				Color4ub(0, 255, 255)
			);
			primitiveRenderer->drawLine(
				body1Center,
				anchor,
				Color4ub(255, 255, 255)
			);
			if (entity2)
			{
				Vector4 body2Center = entity2->getTransform().translation();
				primitiveRenderer->drawLine(
					body2Center,
					anchor,
					Color4ub(255, 255, 255)
				);
			}

			float coneAngle1, coneAngle2;
			coneTwistJointDesc->getConeAngles(coneAngle1, coneAngle2);

			Vector4 bac1 = coneTwistJointDesc->getTwistAxis().normalized();
			Vector4 coneAxis1 = coneTwistJointDesc->getConeAxis().normalized();
			Vector4 coneAxis2 = cross(bac1, coneAxis1).normalized();

			primitiveRenderer->drawCone(
				Matrix44(
					coneAxis1,
					coneAxis2,
					bac1,
					anchor
				),
				coneAngle1,
				coneAngle2,
				c_limitLength,
				Color4ub(255, 255, 0, 220),
				Color4ub(255, 255, 0, 128)
			);

			float twistAngle = coneTwistJointDesc->getTwistAngle() / 2.0f;

			primitiveRenderer->drawProtractor(
				anchor,
				coneAxis1,
				coneAxis2,
				-twistAngle,
				twistAngle,
				deg2rad(4.0f),
				2.0f,
				Color4ub(255, 255, 0, 220),
				Color4ub(255, 255, 0, 128)
			);
		}
		else if (const HingeJointDesc* hingeJointDesc = dynamic_type_cast< const HingeJointDesc* >(constraint.jointDesc))
		{
			const Scalar c_axisLength(5.0f);
			const float c_limitLength = 2.0f;
			const float c_angleStep = 10.0f;

			Vector4 anchor = jointTransform * hingeJointDesc->getAnchor().xyz1();
			Vector4 axis = jointTransform * hingeJointDesc->getAxis().xyz0();

			primitiveRenderer->drawSolidPoint(anchor, 4.0f, Color4ub(255, 255, 255));

			primitiveRenderer->drawLine(
				anchor - axis * c_axisLength,
				anchor + axis * c_axisLength,
				Color4ub(255, 255, 255)
			);

			primitiveRenderer->drawLine(
				body1Center,
				anchor,
				Color4ub(255, 255, 255)
			);
			if (entity2)
			{
				Vector4 body2Center = entity2->getTransform().translation();
				primitiveRenderer->drawLine(
					body2Center,
					anchor,
					Color4ub(255, 255, 255)
				);
			}

			float minAngle, maxAngle;
			hingeJointDesc->getAngles(minAngle, maxAngle);

			Vector4 u = (anchor - body1Center).normalized();
			Vector4 v = cross(axis, u).normalized();

			primitiveRenderer->drawProtractor(
				anchor,
				u,
				v,
				minAngle,
				maxAngle,
				deg2rad(c_angleStep),
				c_limitLength,
				Color4ub(255, 255, 0, 220),
				Color4ub(255, 255, 0, 128)
			);
		}
		else if (const Hinge2JointDesc* hinge2JointDesc = dynamic_type_cast< const Hinge2JointDesc* >(constraint.jointDesc))
		{
			const Scalar c_axisLength(200.0f);

			Vector4 anchor = jointTransform * hinge2JointDesc->getAnchor().xyz1();
			Vector4 axis1 = jointTransform * hinge2JointDesc->getAxis1().xyz0();
			Vector4 axis2 = jointTransform * hinge2JointDesc->getAxis2().xyz0();

			primitiveRenderer->drawSolidPoint(anchor, 4.0f, Color4ub(255, 255, 255));

			primitiveRenderer->drawLine(
				anchor - axis1 * c_axisLength,
				anchor + axis1 * c_axisLength,
				Color4ub(255, 255, 255)
			);
			primitiveRenderer->drawLine(
				anchor - axis2 * c_axisLength,
				anchor + axis2 * c_axisLength,
				Color4ub(255, 255, 255)
			);
		}
		*/
	}
}

	}
}
