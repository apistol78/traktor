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
	if (const RigidBodyComponentData* rigidBodyComponent = dynamic_type_cast< const RigidBodyComponentData* >(m_componentData))
	{
		if (m_context->shouldDrawGuide(L"Physics.Shapes"))
		{
			const BodyDesc* bodyDesc = rigidBodyComponent->getBodyDesc();
			if (bodyDesc)
			{
				const ShapeDesc* shapeDesc = bodyDesc->getShape();
				if (shapeDesc)
				{
					Transform body1Transform0 = m_entityAdapter->getTransform0();
					Transform body1Transform = m_entityAdapter->getTransform();

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
}

	}
}
