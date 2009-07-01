#include "Physics/Editor/ArticulatedEntityEditor.h"
#include "Physics/World/ArticulatedEntityData.h"
#include "Physics/World/ArticulatedEntity.h"
#include "Physics/BallJointDesc.h"
#include "Physics/ConeTwistJointDesc.h"
#include "Physics/HingeJointDesc.h"
#include "Physics/Hinge2JointDesc.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "World/Entity/SpatialEntityData.h"
#include "World/Entity/SpatialEntity.h"
#include "Render/PrimitiveRenderer.h"
#include "Ui/Command.h"
#include "Core/Math/Const.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.ArticulatedEntityEditor", ArticulatedEntityEditor, scene::DefaultEntityEditor)

TypeSet ArticulatedEntityEditor::getEntityTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< ArticulatedEntityData >());
	return typeSet;
}

void ArticulatedEntityEditor::entitySelected(
	scene::SceneEditorContext* context,
	scene::EntityAdapter* entityAdapter,
	bool selected
) const
{
}

bool ArticulatedEntityEditor::handleCommand(
	scene::SceneEditorContext* context,
	scene::EntityAdapter* entityAdapter,
	const ui::Command& command
) const
{
	return false;
}

void ArticulatedEntityEditor::drawGuide(
	scene::SceneEditorContext* context,
	render::PrimitiveRenderer* primitiveRenderer,
	scene::EntityAdapter* entityAdapter
) const
{
	ArticulatedEntityData* articulatedEntityData = checked_type_cast< ArticulatedEntityData* >(entityAdapter->getEntityData());
	const RefArray< world::EntityInstance >& instances = articulatedEntityData->getInstances();
	const std::vector< ArticulatedEntityData::Constraint >& constraints = articulatedEntityData->getConstraints();

	Matrix44 transform = articulatedEntityData->getTransform();

	for (uint32_t i = 0; i < uint32_t(constraints.size()); ++i)
	{
		const ArticulatedEntityData::Constraint& constraint = constraints[i];

		// Let invalid constraint pass for now as user might correct this later.
		if (constraint.entityIndex1 < 0)
			continue;

		Ref< world::EntityInstance > instance1 = instances[constraint.entityIndex1];
		Ref< world::EntityInstance > instance2 = constraint.entityIndex2 >= 0 ? instances[constraint.entityIndex2] : 0;

		Ref< scene::EntityAdapter > entity1 = context->findAdapterFromInstance(instance1);
		Ref< scene::EntityAdapter > entity2 = instance2 ? context->findAdapterFromInstance(instance2) : 0;

		Matrix44 body1OriginalTransformInv = entity1->getTransform().inverseOrtho();
		Matrix44 body1Transform = entity1->getTransform();
		Matrix44 jointTransform = transform * body1OriginalTransformInv * body1Transform;
		Vector4 body1Center = body1Transform.translation();

		if (const BallJointDesc* ballJointDesc = dynamic_type_cast< const BallJointDesc* >(constraint.jointDesc))
		{
			const Vector4 c_knobSize(0.1f, 0.1f, 0.1f, 0.0f);

			Vector4 anchor = jointTransform * ballJointDesc->getAnchor().xyz1();

			primitiveRenderer->drawSolidAabb(
				Aabb(
					anchor - c_knobSize,
					anchor + c_knobSize
				),
				Color(255, 255, 255)
			);

			primitiveRenderer->drawLine(
				body1Center,
				anchor,
				Color(255, 255, 255)
			);

			if (entity2)
			{
				Vector4 body2Center = entity2->getTransform().translation();
				primitiveRenderer->drawLine(
					body2Center,
					anchor,
					Color(255, 255, 255)
				);
			}
		}
		else if (const ConeTwistJointDesc* coneTwistJointDesc = dynamic_type_cast< const ConeTwistJointDesc* >(constraint.jointDesc))
		{
			const Vector4 c_knobSize(0.1f, 0.1f, 0.1f, 0.0f);
			const Scalar c_axisLength(5.0f);
			const float c_limitLength = 2.0f;

			Vector4 anchor = jointTransform * coneTwistJointDesc->getAnchor().xyz1();

			primitiveRenderer->drawSolidAabb(
				Aabb(
					anchor - c_knobSize,
					anchor + c_knobSize
				),
				Color(255, 255, 255)
			);

			primitiveRenderer->drawLine(
				anchor - coneTwistJointDesc->getTwistAxis() * c_axisLength,
				anchor + coneTwistJointDesc->getTwistAxis() * c_axisLength,
				Color(255, 255, 0)
			);
			primitiveRenderer->drawLine(
				anchor - coneTwistJointDesc->getConeAxis() * c_axisLength,
				anchor + coneTwistJointDesc->getConeAxis() * c_axisLength,
				Color(0, 255, 255)
			);
			primitiveRenderer->drawLine(
				body1Center,
				anchor,
				Color(255, 255, 255)
			);
			if (entity2)
			{
				Vector4 body2Center = entity2->getTransform().translation();
				primitiveRenderer->drawLine(
					body2Center,
					anchor,
					Color(255, 255, 255)
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
				Color(255, 255, 0, 220),
				Color(255, 255, 0, 128)
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
				Color(255, 255, 0, 220),
				Color(255, 255, 0, 128)
			);
		}
		else if (const HingeJointDesc* hingeJointDesc = dynamic_type_cast< const HingeJointDesc* >(constraint.jointDesc))
		{
			const Vector4 c_knobSize(0.1f, 0.1f, 0.1f, 0.0f);
			const Scalar c_axisLength(5.0f);
			const float c_limitLength = 2.0f;
			const float c_angleStep = 10.0f;

			Vector4 anchor = jointTransform * hingeJointDesc->getAnchor().xyz1();
			Vector4 axis = jointTransform * hingeJointDesc->getAxis().xyz0();

			primitiveRenderer->drawSolidAabb(
				Aabb(
					anchor - c_knobSize,
					anchor + c_knobSize
				),
				Color(255, 255, 255)
			);

			primitiveRenderer->drawLine(
				anchor - axis * c_axisLength,
				anchor + axis * c_axisLength,
				Color(255, 255, 255)
			);

			primitiveRenderer->drawLine(
				body1Center,
				anchor,
				Color(255, 255, 255)
			);
			if (entity2)
			{
				Vector4 body2Center = entity2->getTransform().translation();
				primitiveRenderer->drawLine(
					body2Center,
					anchor,
					Color(255, 255, 255)
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
				Color(255, 255, 0, 220),
				Color(255, 255, 0, 128)
			);
		}
		else if (const Hinge2JointDesc* hinge2JointDesc = dynamic_type_cast< const Hinge2JointDesc* >(constraint.jointDesc))
		{
			const Vector4 c_knobSize(0.1f, 0.1f, 0.1f, 0.0f);
			const Scalar c_axisLength(200.0f);

			Vector4 anchor = jointTransform * hinge2JointDesc->getAnchor().xyz1();
			Vector4 axis1 = jointTransform * hinge2JointDesc->getAxis1().xyz0();
			Vector4 axis2 = jointTransform * hinge2JointDesc->getAxis2().xyz0();

			primitiveRenderer->drawSolidAabb(
				Aabb(
					anchor - c_knobSize,
					anchor + c_knobSize
				),
				Color(255, 255, 255)
			);

			primitiveRenderer->drawLine(
				anchor - axis1 * c_axisLength,
				anchor + axis1 * c_axisLength,
				Color(255, 255, 255)
			);
			primitiveRenderer->drawLine(
				anchor - axis2 * c_axisLength,
				anchor + axis2 * c_axisLength,
				Color(255, 255, 255)
			);
		}
	}

	for (RefArray< world::EntityInstance >::const_iterator i = instances.begin(); i != instances.end(); ++i)
	{
		Ref< scene::EntityAdapter > entityAdapter = context->findAdapterFromInstance(*i);
		if (entityAdapter)
			context->drawGuide(primitiveRenderer, entityAdapter);
	}
}

	}
}
