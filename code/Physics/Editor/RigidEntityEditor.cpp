#include "Physics/Editor/RigidEntityEditor.h"
#include "Physics/World/RigidEntityData.h"
#include "Physics/World/RigidEntity.h"
#include "Physics/DynamicBody.h"
#include "Physics/BodyDesc.h"
#include "Physics/BoxShapeDesc.h"
#include "Physics/CapsuleShapeDesc.h"
#include "Physics/CylinderShapeDesc.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/SphereShapeDesc.h"
#include "Physics/HeightfieldShapeDesc.h"
#include "Physics/Mesh.h"
#include "Physics/Heightfield.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "World/Entity/SpatialEntity.h"
#include "Render/PrimitiveRenderer.h"
#include "Resource/IResourceManager.h"
#include "Ui/Command.h"
#include "Core/Math/Const.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.RigidEntityEditor", RigidEntityEditor, scene::DefaultEntityEditor)

RigidEntityEditor::RigidEntityEditor(scene::SceneEditorContext* context)
:	scene::DefaultEntityEditor(context)
,	m_showHull(false)
{
}

void RigidEntityEditor::entitySelected(
	scene::SceneEditorContext* context,
	scene::EntityAdapter* entityAdapter,
	bool selected
)
{
	if (selected)
		m_showHull = false;
}

void RigidEntityEditor::applyModifier(
	scene::SceneEditorContext* context,
	scene::EntityAdapter* entityAdapter,
	const Matrix44& viewTransform,
	const Vector4& screenDelta,
	const Vector4& viewDelta,
	const Vector4& worldDelta,
	int mouseButton
)
{
	// Apply default modifier.
	scene::DefaultEntityEditor::applyModifier(
		context,
		entityAdapter,
		viewTransform,
		screenDelta,
		viewDelta,
		worldDelta,
		mouseButton
	);

	// Ensure body is enabled as it might have gone to sleep.
	Ref< RigidEntity > rigidEntity = checked_type_cast< RigidEntity* >(entityAdapter->getEntity());
	if (rigidEntity->getBody())
		rigidEntity->getBody()->setActive(true);
}

bool RigidEntityEditor::handleCommand(
	scene::SceneEditorContext* context,
	scene::EntityAdapter* entityAdapter,
	const ui::Command& command
)
{
	if (command == L"Physics.ToggleMeshTriangles")
		m_showHull = !m_showHull;
	else
		return false;

	return true;
}

void RigidEntityEditor::drawGuide(
	scene::SceneEditorContext* context,
	render::PrimitiveRenderer* primitiveRenderer,
	scene::EntityAdapter* entityAdapter
) const
{
	Ref< RigidEntityData > rigidEntityData = checked_type_cast< RigidEntityData* >(entityAdapter->getEntityData());
	Ref< RigidEntity > rigidEntity = checked_type_cast< RigidEntity* >(entityAdapter->getEntity());

	primitiveRenderer->pushWorld(entityAdapter->getTransform().toMatrix44());

	// Draw collision shape geometry.
	Ref< const BodyDesc > bodyDesc = rigidEntityData->getBodyDesc();
	if (bodyDesc)
	{
		Ref< const ShapeDesc > shapeDesc = bodyDesc->getShape();
		if (shapeDesc)
		{
			primitiveRenderer->pushWorld(primitiveRenderer->getWorld() * shapeDesc->getLocalTransform().toMatrix44());

			if (const BoxShapeDesc* boxShapeDesc = dynamic_type_cast< const BoxShapeDesc* >(shapeDesc))
			{
				Aabb boundingBox(-boxShapeDesc->getExtent(), boxShapeDesc->getExtent());

				if (entityAdapter->isSelected())
				{
					primitiveRenderer->drawSolidAabb(boundingBox, Color(128, 255, 255, 128));
					primitiveRenderer->drawWireAabb(boundingBox, Color(0, 255, 255));
				}
				else
					primitiveRenderer->drawWireAabb(boundingBox, Color(0, 255, 255, 180));
			}
			else if (const CapsuleShapeDesc* capsuleShapeDesc = dynamic_type_cast< const CapsuleShapeDesc* >(shapeDesc))
			{
				Vector4 extent(capsuleShapeDesc->getRadius(), capsuleShapeDesc->getRadius(), capsuleShapeDesc->getLength() * 0.5f);
				Aabb boundingBox(-extent, extent);

				if (entityAdapter->isSelected())
				{
					primitiveRenderer->drawSolidAabb(boundingBox, Color(128, 255, 255, 128));
					primitiveRenderer->drawWireAabb(boundingBox, Color(0, 255, 255));
				}
				else
					primitiveRenderer->drawWireAabb(boundingBox, Color(0, 255, 255, 180));
			}
			else if (const CylinderShapeDesc* cylinderShapeDesc = dynamic_type_cast< const CylinderShapeDesc* >(shapeDesc))
			{
				Vector4 extent(cylinderShapeDesc->getRadius(), cylinderShapeDesc->getRadius(), cylinderShapeDesc->getLength() * 0.5f);
				Aabb boundingBox(-extent, extent);

				if (entityAdapter->isSelected())
				{
					primitiveRenderer->drawSolidAabb(boundingBox, Color(128, 255, 255, 128));
					primitiveRenderer->drawWireCylinder(
						Matrix44::identity(),
						cylinderShapeDesc->getRadius(),
						cylinderShapeDesc->getLength(),
						Color(0, 255, 255)
					);
				}
				else
					primitiveRenderer->drawWireCylinder(
						Matrix44::identity(),
						cylinderShapeDesc->getRadius(),
						cylinderShapeDesc->getLength(),
						Color(0, 255, 255, 180)
					);
			}
			else if (const MeshShapeDesc* meshShapeDesc = dynamic_type_cast< const MeshShapeDesc* >(shapeDesc))
			{
				resource::Proxy< Mesh > mesh = meshShapeDesc->getMesh();
				if (context->getResourceManager()->bind(mesh))
				{
					const AlignedVector< Vector4 >& vertices = mesh->getVertices();
					const std::vector< Mesh::Triangle >& triangles = 
						m_showHull ?
						mesh->getHullTriangles() :
						mesh->getShapeTriangles();

					for (std::vector< Mesh::Triangle >::const_iterator i = triangles.begin(); i != triangles.end(); ++i)
					{
						const Vector4& V0 = vertices[i->indices[0]];
						const Vector4& V1 = vertices[i->indices[1]];
						const Vector4& V2 = vertices[i->indices[2]];

						if (entityAdapter->isSelected())
						{
							primitiveRenderer->drawSolidTriangle(V0, V1, V2, Color(128, 255, 255, 128));
							primitiveRenderer->drawWireTriangle(V0, V1, V2, Color(0, 255, 255));
						}
						else
							primitiveRenderer->drawWireTriangle(V0, V1, V2, Color(0, 255, 255, 180));
					}
				}
			}
			else if (const SphereShapeDesc* sphereShapeDesc = dynamic_type_cast< const SphereShapeDesc* >(shapeDesc))
			{
				float radius = sphereShapeDesc->getRadius();
				Aabb boundingBox(-Vector4(radius, radius, radius, 0.0f), Vector4(radius, radius, radius, 0.0f));

				if (entityAdapter->isSelected())
				{
					primitiveRenderer->drawSolidAabb(boundingBox, Color(128, 255, 255, 128));
					primitiveRenderer->drawWireAabb(boundingBox, Color(0, 255, 255));
				}
				else
					primitiveRenderer->drawWireAabb(boundingBox, Color(0, 255, 255, 180));
			}
			else if (const HeightfieldShapeDesc* heightfieldShapeDesc = dynamic_type_cast< const HeightfieldShapeDesc* >(shapeDesc))
			{
				resource::Proxy< Heightfield > heightfield = heightfieldShapeDesc->getHeightfield();
				if (context->getResourceManager()->bind(heightfield))
				{
					const Vector4& extent = heightfield->getWorldExtent();
					Aabb boundingBox(-extent / Scalar(2.0f), extent / Scalar(2.0f));

					if (entityAdapter->isSelected())
					{
						primitiveRenderer->drawSolidAabb(boundingBox, Color(128, 255, 255, 128));
						primitiveRenderer->drawWireAabb(boundingBox, Color(0, 255, 255));
					}
					else
						primitiveRenderer->drawWireAabb(boundingBox, Color(0, 255, 255, 180));
				}
			}

			primitiveRenderer->popWorld();
		}
	}

	primitiveRenderer->popWorld();

	// Draw default guides of contained entity.
	const RefArray< scene::EntityAdapter >& children = entityAdapter->getChildren();
	for (RefArray< scene::EntityAdapter >::const_iterator i = children.begin(); i != children.end(); ++i)
	{
		if (*i)
			context->drawGuide(primitiveRenderer, *i);
	}
}

	}
}
