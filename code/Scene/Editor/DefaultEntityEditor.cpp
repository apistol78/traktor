#include <limits>
#include "Core/Math/Const.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmArray.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/IEditor.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/CameraMesh.h"
#include "Scene/Editor/DefaultEntityEditor.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/IModifier.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"
#include "World/Entity/CameraEntity.h"
#include "World/Entity/DirectionalLightEntity.h"
#include "World/Entity/GroupEntityData.h"
#include "World/Entity/PointLightEntity.h"
#include "World/Entity/SpotLightEntity.h"
#include "World/Entity/VolumeEntity.h"
#include "World/Entity/VolumeEntityData.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.DefaultEntityEditor", DefaultEntityEditor, IEntityEditor)

DefaultEntityEditor::DefaultEntityEditor(SceneEditorContext* context, EntityAdapter* entityAdapter)
:	m_context(context)
,	m_entityAdapter(entityAdapter)
,	m_inModify(false)
{
	updateSettings();
}

bool DefaultEntityEditor::isPickable() const
{
	if (
		is_a< world::DirectionalLightEntity >(m_entityAdapter->getEntity()) ||
		is_a< world::PointLightEntity >(m_entityAdapter->getEntity()) ||
		is_a< world::SpotLightEntity >(m_entityAdapter->getEntity()) ||
		is_a< world::VolumeEntity >(m_entityAdapter->getEntity())
	)
		return false;
	else
		return !isGroup();
}

bool DefaultEntityEditor::isGroup() const
{
	const world::EntityData* entityData = m_entityAdapter->getEntityData();
	return is_a< world::GroupEntityData >(entityData);
}

bool DefaultEntityEditor::isChildrenPrivate() const
{
	return false;
}

bool DefaultEntityEditor::addChildEntity(EntityAdapter* childEntityAdapter) const
{
	world::EntityData* entityData = m_entityAdapter->getEntityData();
	world::EntityData* childEntityData = childEntityAdapter->getEntityData();

	if (world::GroupEntityData* groupEntityData = dynamic_type_cast< world::GroupEntityData* >(entityData))
	{
		groupEntityData->addEntityData(childEntityData);
		return true;
	}

	return false;
}

bool DefaultEntityEditor::removeChildEntity(EntityAdapter* childEntityAdapter) const
{
	world::EntityData* entityData = m_entityAdapter->getEntityData();
	world::EntityData* childEntityData = childEntityAdapter->getEntityData();

	Ref< Reflection > r = Reflection::create(entityData);
	T_ASSERT (r);

	uint32_t removedCount = 0;
	for (uint32_t j = 0; j < r->getMemberCount(); ++j)
	{
		RfmObject* ro = dynamic_type_cast< RfmObject* >(r->getMember(j));
		if (ro != 0 && ro->get() == childEntityData)
		{
			ro->set(0);
			removedCount++;
		}

		RfmArray* ra = dynamic_type_cast< RfmArray* >(r->getMember(j));
		if (ra != 0)
		{
			for (uint32_t k = 0; k < ra->getMemberCount(); ++k)
			{
				RfmObject* ro = dynamic_type_cast< RfmObject* >(ra->getMember(k));
				if (ro != 0 && ro->get() == childEntityData)
				{
					ra->removeMember(ro);
					removedCount++;
				}
			}
		}
	}

	if (removedCount > 0)
	{
		r->apply(entityData);
		return true;
	}

	return false;
}

bool DefaultEntityEditor::queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection, Scalar& outDistance) const
{
	// Transform ray into object space.
	Transform worldInv = m_entityAdapter->getTransform().inverse();
	Vector4 objectRayOrigin = worldInv * worldRayOrigin.xyz1();
	Vector4 objectRayDirection = worldInv * worldRayDirection.xyz0();

	// Get entity bounding box; do not pick if origin of ray is within box.
	Aabb3 boundingBox = m_entityAdapter->getBoundingBox();
	if (boundingBox.empty() || boundingBox.inside(objectRayOrigin))
		return false;

	// Trace bounding box to see if ray intersect.
	Scalar length = outDistance - Scalar(FUZZY_EPSILON);
	Scalar distance;
	if (!boundingBox.intersectSegment(objectRayOrigin, objectRayOrigin + objectRayDirection * length, distance))
		return false;

	T_FATAL_ASSERT (distance <= 1.0f);
	outDistance = distance * length;
	return true;
}

bool DefaultEntityEditor::queryFrustum(const Frustum& worldFrustum) const
{
	// Transform frustum into object space.
	Transform worldInv = m_entityAdapter->getTransform().inverse();
	Plane objectPlanes[6];
	for (int32_t i = 0; i < 6; ++i)
		objectPlanes[i] = worldInv.toMatrix44() * worldFrustum.planes[i];
	
	// Get entity bounding box.
	Aabb3 boundingBox = m_entityAdapter->getBoundingBox();
	if (boundingBox.empty())
		return false;

	Frustum objectFrustum;
	objectFrustum.buildFromPlanes(objectPlanes);
	return objectFrustum.inside(boundingBox) != Frustum::IrOutside;
}

void DefaultEntityEditor::entitySelected(bool selected)
{
}

bool DefaultEntityEditor::handleCommand(const ui::Command& command)
{
	if (command == L"Editor.SettingsChanged")
		updateSettings();
	return false;
}

void DefaultEntityEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	const Vector4 c_expandBoundingBox(0.001f, 0.001f, 0.001f, 0.0f);

	Transform transform = m_entityAdapter->getTransform();
	
	Aabb3 boundingBox = m_entityAdapter->getBoundingBox();
	boundingBox.mn -= c_expandBoundingBox;
	boundingBox.mx += c_expandBoundingBox;

	if (is_a< world::CameraEntity >(m_entityAdapter->getEntity()))
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
	else if (is_a< world::DirectionalLightEntity >(m_entityAdapter->getEntity()))
	{
		if (m_context->shouldDrawGuide(L"Entity.Light"))
		{
			Vector4 lightPosition = transform.translation();
			Vector4 lightDirection = -transform.axisY();
			Vector4 lightX = transform.axisX();
			Vector4 lightZ = transform.axisZ();

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

			primitiveRenderer->pushWorld(transform.toMatrix44());
			primitiveRenderer->drawWireAabb(Aabb3(Vector4(-0.25f, -0.25f, -0.25f, 1.0f), Vector4(0.25f, 0.25f, 0.25f, 1.0f)), m_colorBoundingBox);
			primitiveRenderer->popWorld();
		}
	}
	else if (const world::VolumeEntityData* volumeEntity = dynamic_type_cast< const world::VolumeEntityData* >(m_entityAdapter->getEntityData()))
	{
		if (m_context->shouldDrawGuide(L"Entity.Volumes"))
		{
			Transform T = getEntityAdapter()->getTransform();
			primitiveRenderer->pushWorld(T.toMatrix44());
			const AlignedVector< Aabb3 >& volumes = volumeEntity->getVolumes();
			for (AlignedVector< Aabb3 >::const_iterator i = volumes.begin(); i != volumes.end(); ++i)
			{
				primitiveRenderer->drawSolidAabb(*i, Color4ub(120, 255, 120, 80));
				primitiveRenderer->drawWireAabb(*i, Color4ub(120, 255, 120, 255));
			}
			primitiveRenderer->popWorld();
		}
	}
	else
	{
		if (!m_entityAdapter->getParent())
			return;

		if (m_context->shouldDrawGuide(L"Entity.BoundingBox"))
		{
			primitiveRenderer->pushWorld(transform.toMatrix44());
			if (m_entityAdapter->isSelected())
			{
				primitiveRenderer->drawSolidAabb(boundingBox, m_colorBoundingBoxFaceSel);
				primitiveRenderer->drawWireAabb(boundingBox, m_colorBoundingBoxSel);
			}
			else
				primitiveRenderer->drawWireAabb(boundingBox, m_colorBoundingBox);
			primitiveRenderer->popWorld();

			if (m_entityAdapter->isSelected() && m_context->getSnapMode() == SceneEditorContext::SmNeighbour)
			{
				AlignedVector< EntityAdapter::SnapPoint > snapPoints = m_entityAdapter->getSnapPoints();
				for (AlignedVector< EntityAdapter::SnapPoint >::const_iterator i = snapPoints.begin(); i != snapPoints.end(); ++i)
				{
					primitiveRenderer->drawSolidPoint(
						i->position,
						4.0f,
						m_colorSnap
					);
				}
			}
		}
	}
}

bool DefaultEntityEditor::getStatusText(std::wstring& outStatusText) const
{
	return false;
}

void DefaultEntityEditor::updateSettings()
{
	Ref< PropertyGroup > colors = m_context->getEditor()->getSettings()->getProperty< PropertyGroup >(L"Editor.Colors");
	m_colorBoundingBox = colors->getProperty< PropertyColor >(L"BoundingBoxWire");
	m_colorBoundingBoxSel = colors->getProperty< PropertyColor >(L"BoundingBoxWireSelected");
	m_colorBoundingBoxFaceSel = colors->getProperty< PropertyColor >(L"BoundingBoxFaceSelected");
	m_colorSnap = colors->getProperty< PropertyColor >(L"SnapPoint");
}

	}
}
