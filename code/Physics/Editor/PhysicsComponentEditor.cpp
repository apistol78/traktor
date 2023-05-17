/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/BodyDesc.h"
#include "Physics/ShapeDesc.h"
#include "Physics/Editor/PhysicsComponentEditor.h"
#include "Physics/World/JointComponentData.h"
#include "Physics/World/RigidBodyComponentData.h"
#include "Physics/World/Character/CharacterComponent.h"
#include "Physics/World/Character/CharacterComponentData.h"
#include "Physics/World/Vehicle/VehicleComponent.h"
#include "Physics/World/Vehicle/VehicleComponentData.h"
#include "Physics/World/Vehicle/Wheel.h"
#include "Physics/World/Vehicle/WheelData.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"

namespace traktor::physics
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.PhysicsComponentEditor", PhysicsComponentEditor, scene::IComponentEditor)

PhysicsComponentEditor::PhysicsComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData)
:	m_context(context)
,	m_entityAdapter(entityAdapter)
,	m_componentData(componentData)
{
}

void PhysicsComponentEditor::transformModified(scene::EntityAdapter* entityAdapter, scene::EntityAdapter* modifiedEntityAdapter)
{
}

void PhysicsComponentEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	if (m_context->shouldDrawGuide(L"Physics.Shapes"))
	{
		Transform body1Transform[] = { m_entityAdapter->getTransform0(), m_entityAdapter->getTransform() };

		Ref< const ShapeDesc > shapeDesc;

		if (const RigidBodyComponentData* rigidBodyComponent = dynamic_type_cast< const RigidBodyComponentData* >(m_componentData))
		{
			auto bodyDesc = rigidBodyComponent->getBodyDesc();
			if (bodyDesc)
				shapeDesc = bodyDesc->getShape();
		}

		if (const CharacterComponentData* characterComponent = dynamic_type_cast< const CharacterComponentData* >(m_componentData))
		{
			shapeDesc = characterComponent->getShapeDesc(0.0f);

			body1Transform[0] = body1Transform[0] * Transform(Vector4(0.0f, characterComponent->getHeight() / 2.0f, 0.0f));
			body1Transform[1] = body1Transform[1] * Transform(Vector4(0.0f, characterComponent->getHeight() / 2.0f, 0.0f));
		}

		if (const VehicleComponentData* vehicleComponent = dynamic_type_cast< const VehicleComponentData* >(m_componentData))
		{
			primitiveRenderer->pushWorld(body1Transform[1].toMatrix44());

			auto vehicle = m_entityAdapter->getComponent< VehicleComponent >();
			if (vehicle)
			{
				const auto& wheels = vehicle->getWheels();
				for (auto wheel : wheels)
				{
					if (wheel->contact)
						primitiveRenderer->drawSolidPoint(body1Transform[1].inverse() * wheel->contactPosition, 8.0f, Color4ub(255, 255, 0, 255));

					const Vector4 anchorL = wheel->data->getAnchor().xyz1();
					const Vector4 axisL = wheel->data->getAxis().xyz0().normalized();
					const Scalar suspLen = Scalar(wheel->suspensionLength);
					const Vector4 position = anchorL + axisL * -suspLen;

					primitiveRenderer->drawWireCylinder(
						translate(position) * rotateY(deg2rad(90.0f)),
						wheel->data->getRadius(),
						wheel->data->getRadius() / 2.0f,
						Color4ub(0, 255, 255, 255)
					);
				}
			}

			const auto& wheels = vehicleComponent->getWheels();
			for (auto wheel : wheels)
			{
				if (!wheel)
					continue;

				const Vector4 anchorL = wheel->getAnchor().xyz1();
				const Vector4 axisL = wheel->getAxis().xyz0().normalized();
				const Scalar suspMin = Scalar(wheel->getSuspensionLength().min);
				const Scalar suspMax = Scalar(wheel->getSuspensionLength().max);

				primitiveRenderer->drawSolidPoint(anchorL, 8.0f, Color4ub(255, 0, 0, 255));
				primitiveRenderer->drawSolidPoint(anchorL - axisL * suspMin, 8.0f, Color4ub(255, 0, 0, 255));
				primitiveRenderer->drawSolidPoint(anchorL - axisL * suspMax, 8.0f, Color4ub(255, 0, 0, 255));
				primitiveRenderer->drawLine(anchorL, anchorL - axisL * suspMax, Color4ub(255, 0, 0, 255));
			}

			primitiveRenderer->popWorld();
		}

		if (shapeDesc)
		{
			m_physicsRenderer.draw(
				m_context->getResourceManager(),
				primitiveRenderer,
				body1Transform,
				shapeDesc
			);
		}
	}

	if (m_context->shouldDrawGuide(L"Physics.Joints"))
	{
		Transform jointTransform[] = { m_entityAdapter->getTransform0(), m_entityAdapter->getTransform() };
		Transform body1Transform[] = { Transform::identity(), Transform::identity() };
		Transform body2Transform[] = { Transform::identity(), Transform::identity() };

		if (const JointComponentData* jointComponent = dynamic_type_cast< const JointComponentData* >(m_componentData))
		{
			RefArray< scene::EntityAdapter > bodyEntities;
			for (auto child : m_entityAdapter->getChildren())
			{
				if (child->getComponentData< RigidBodyComponentData >() != nullptr)
					bodyEntities.push_back(child);
			}
			if (bodyEntities.size() >= 1)
			{
				body1Transform[0] = bodyEntities[0]->getTransform0();
				body1Transform[1] = bodyEntities[0]->getTransform();
			}
			if (bodyEntities.size() >= 2)
			{
				body2Transform[0] = bodyEntities[1]->getTransform0();
				body2Transform[1] = bodyEntities[1]->getTransform();
				jointTransform[1] = body1Transform[1] * body1Transform[0].inverse() * jointTransform[0];
			}

			auto jointDesc = jointComponent->getJointDesc();
			if (jointDesc != nullptr)
			{
				m_physicsRenderer.draw(
					primitiveRenderer,
					jointTransform,
					body1Transform,
					body2Transform,
					jointDesc
				);
			}
		}
	}
}

}
