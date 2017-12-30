/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Physics/BodyDesc.h"
#include "Physics/ShapeDesc.h"
#include "Physics/Editor/PhysicsComponentEditor.h"
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

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.PhysicsComponentEditor", PhysicsComponentEditor, scene::IComponentEditor)

PhysicsComponentEditor::PhysicsComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData)
:	m_context(context)
,	m_entityAdapter(entityAdapter)
,	m_componentData(componentData)
{
}

void PhysicsComponentEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	if (!m_context->shouldDrawGuide(L"Physics.Shapes"))
		return;

	Transform body1Transform0 = m_entityAdapter->getTransform0();
	Transform body1Transform = m_entityAdapter->getTransform();

	const ShapeDesc* shapeDesc = nullptr;

	if (const RigidBodyComponentData* rigidBodyComponent = dynamic_type_cast< const RigidBodyComponentData* >(m_componentData))
	{
		auto bodyDesc = rigidBodyComponent->getBodyDesc();
		if (bodyDesc)
			shapeDesc = bodyDesc->getShape();
	}

	if (const CharacterComponentData* characterComponent = dynamic_type_cast< const CharacterComponentData* >(m_componentData))
	{
		auto bodyDesc = characterComponent->getBodyDesc();
		if (bodyDesc)
			shapeDesc = bodyDesc->getShape();
	}

	if (const VehicleComponentData* vehicleComponent = dynamic_type_cast< const VehicleComponentData* >(m_componentData))
	{
		auto bodyDesc = vehicleComponent->getBodyDesc();
		if (bodyDesc)
			shapeDesc = bodyDesc->getShape();

		primitiveRenderer->pushWorld(body1Transform.toMatrix44());

		auto vehicle = m_entityAdapter->getComponent< VehicleComponent >();
		if (vehicle)
		{
			const auto& wheels = vehicle->getWheels();
			for (auto wheel : wheels)
			{
				if (wheel->contact)
					primitiveRenderer->drawSolidPoint(body1Transform.inverse() * wheel->contactPosition, 8.0f, Color4ub(255, 255, 0, 255));

				Vector4 anchorL = wheel->data->getAnchor().xyz1();
				Vector4 axisL = wheel->data->getAxis().xyz0().normalized();
				Scalar suspLen = Scalar(wheel->suspensionFilteredLength);

				Vector4 position = anchorL + axisL * -suspLen;

				primitiveRenderer->drawWireCylinder(
					translate(position),
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

			Vector4 anchorL = wheel->getAnchor().xyz1();
			Vector4 axisL = wheel->getAxis().xyz0().normalized();
			Scalar suspMin = Scalar(wheel->getSuspensionLength().min);
			Scalar suspMax = Scalar(wheel->getSuspensionLength().max);

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
			body1Transform0,
			body1Transform,
			shapeDesc
		);
	}
}

	}
}
