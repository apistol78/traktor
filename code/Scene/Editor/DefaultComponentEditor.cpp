/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/CameraMesh.h"
#include "Scene/Editor/DefaultComponentEditor.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "World/Entity/CameraComponentData.h"
#include "World/Entity/LightComponentData.h"
#include "World/Entity/VolumeComponentData.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.DefaultComponentEditor", DefaultComponentEditor, IComponentEditor)

DefaultComponentEditor::DefaultComponentEditor(SceneEditorContext* context, EntityAdapter* entityAdapter, world::IEntityComponentData* componentData)
:	m_context(context)
,	m_entityAdapter(entityAdapter)
,	m_componentData(componentData)
{
}

void DefaultComponentEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	Transform transform = m_entityAdapter->getTransform();

	if (is_a< world::CameraComponentData >(m_componentData))
	{
		primitiveRenderer->pushWorld(transform.toMatrix44());
		primitiveRenderer->pushDepthState(false, false, false);

		primitiveRenderer->drawWireAabb(
			Vector4::origo(),
			Vector4(0.1f, 0.1f, 0.1f, 0.0f),
			Color4ub(255, 255, 255, 255)
		);

		for (int j = 0; j < sizeof_array(c_cameraMeshIndices); j += 2)
		{
			int32_t i1 = c_cameraMeshIndices[j + 0] - 1;
			int32_t i2 = c_cameraMeshIndices[j + 1] - 1;

			const float* v1 = &c_cameraMeshVertices[i1 * 3];
			const float* v2 = &c_cameraMeshVertices[i2 * 3];

			primitiveRenderer->drawLine(
				Vector4(v1[0], v1[1], v1[2], 1.0f),
				Vector4(v2[0], v2[1], v2[2], 1.0f),
				Color4ub(255, 255, 255, 255)
			);
		}

		primitiveRenderer->popDepthState();
		primitiveRenderer->popWorld();
	}

	if (const world::LightComponentData* lightComponent = dynamic_type_cast< const world::LightComponentData* >(m_componentData))
	{
		if (m_context->shouldDrawGuide(L"Entity.Light"))
		{
			Vector4 lightPosition = transform.translation();
			Vector4 lightDirection = -transform.axisY();
			Vector4 lightX = transform.axisX();
			Vector4 lightZ = transform.axisZ();

			primitiveRenderer->pushDepthState(true, true, false);

			primitiveRenderer->drawLine(
				lightPosition - lightDirection * Scalar(0.5f),
				lightPosition + lightDirection * Scalar(0.5f),
				5.0f,
				Color4ub(255, 255, 0)
			);
			primitiveRenderer->drawArrowHead(
				lightPosition + lightDirection * Scalar(0.5f),
				lightPosition + lightDirection * Scalar(0.7f),
				0.5f,
				Color4ub(255, 255, 0)
			);

			primitiveRenderer->popDepthState();

			primitiveRenderer->pushWorld(transform.toMatrix44());
			primitiveRenderer->drawWireAabb(Aabb3(Vector4(-0.25f, -0.25f, -0.25f, 1.0f), Vector4(0.25f, 0.25f, 0.25f, 1.0f)), Color4ub(120, 255, 120, 255));
			primitiveRenderer->popWorld();
		}
	}

	if (const world::VolumeComponentData* volumeComponent = dynamic_type_cast< const world::VolumeComponentData* >(m_componentData))
	{
		if (m_context->shouldDrawGuide(L"Entity.Volumes"))
		{
			primitiveRenderer->pushWorld(transform.toMatrix44());

			const AlignedVector< Aabb3 >& volumes = volumeComponent->getVolumes();
			for (AlignedVector< Aabb3 >::const_iterator i = volumes.begin(); i != volumes.end(); ++i)
			{
				primitiveRenderer->drawSolidAabb(*i, Color4ub(120, 255, 120, 80));
				primitiveRenderer->drawWireAabb(*i, Color4ub(120, 255, 120, 255));
			}

			primitiveRenderer->popWorld();
		}
	}
}

	}
}
