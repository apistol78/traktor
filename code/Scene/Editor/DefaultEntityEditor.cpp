#include <limits>
#include "Core/Math/Const.h"
#include "Core/Reflection/Reflection.h"
#include "Core/Reflection/RfmArray.h"
#include "Core/Reflection/RfmObject.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyGroup.h"
#include "Editor/IEditor.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/DefaultEntityEditor.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/IModifier.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"
#include "World/EntityData.h"
#include "World/Entity/GroupComponentData.h"
#include "World/Entity/GroupEntityData.h"
#include "World/Entity/LightComponentData.h"

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
	return !isGroup();
}

bool DefaultEntityEditor::isGroup() const
{
	const world::EntityData* entityData = m_entityAdapter->getEntityData();
	if (is_a< world::GroupEntityData >(entityData))
		return true;
	else
		return (bool)(entityData->getComponent< world::GroupComponentData >() != nullptr);
}

bool DefaultEntityEditor::isChildrenPrivate() const
{
	return false;
}

bool DefaultEntityEditor::addChildEntity(EntityAdapter* childEntityAdapter) const
{
	world::EntityData* entityData = m_entityAdapter->getEntityData();
	world::EntityData* childEntityData = childEntityAdapter->getEntityData();

	if (auto groupEntityData = dynamic_type_cast< world::GroupEntityData* >(entityData))
	{
		groupEntityData->addEntityData(childEntityData);
		return true;
	}
	else
	{
		entityData->getComponent< world::GroupComponentData >()->addEntityData(childEntityData);
		return true;
	}
}

bool DefaultEntityEditor::removeChildEntity(EntityAdapter* childEntityAdapter) const
{
	world::EntityData* entityData = m_entityAdapter->getEntityData();
	world::EntityData* childEntityData = childEntityAdapter->getEntityData();

	Ref< Reflection > r;
	if (auto groupEntityData = dynamic_type_cast< world::GroupEntityData* >(entityData))
		r = Reflection::create(groupEntityData);
	else	
	 	r = Reflection::create(entityData->getComponent< world::GroupComponentData >());

	if (!r)
		return false;

	uint32_t removedCount = 0;
	for (uint32_t j = 0; j < r->getMemberCount(); ++j)
	{
		RfmObject* ro = dynamic_type_cast< RfmObject* >(r->getMember(j));
		if (ro != nullptr && ro->get() == childEntityData)
		{
			ro->set(nullptr);
			removedCount++;
		}

		RfmArray* ra = dynamic_type_cast< RfmArray* >(r->getMember(j));
		if (ra != nullptr)
		{
			for (uint32_t k = 0; k < ra->getMemberCount(); ++k)
			{
				RfmObject* ro = dynamic_type_cast< RfmObject* >(ra->getMember(k));
				if (ro != nullptr && ro->get() == childEntityData)
				{
					ra->removeMember(ro);
					removedCount++;
				}
			}
		}
	}

	if (removedCount > 0)
	{
		if (auto groupEntityData = dynamic_type_cast< world::GroupEntityData* >(entityData))
			r->apply(entityData);
		else
			r->apply(entityData->getComponent< world::GroupComponentData >());
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

	if (auto lightComponentData = m_entityAdapter->getComponentData< world::LightComponentData >())
	{
		if (!m_context->shouldDrawGuide(L"Entity.Light"))
			return;

		Vector4 lightPosition = transform.translation();
		Vector4 lightDirection = -transform.axisY();
		Vector4 lightX = transform.axisX();
		Vector4 lightZ = transform.axisZ();

		primitiveRenderer->pushDepthState(true, true, false);

		if (lightComponentData->getLightType() == world::LtDirectional)
		{
			primitiveRenderer->drawSolidPoint(lightPosition, 8.0f, Color4ub(255, 255, 0));
			primitiveRenderer->drawLine(
				lightPosition,
				lightPosition + lightDirection * Scalar(0.5f),
				5.0f,
				Color4ub(255, 255, 255)
			);
			primitiveRenderer->drawArrowHead(
				lightPosition + lightDirection * Scalar(0.5f),
				lightPosition + lightDirection * Scalar(0.7f),
				0.5f,
				Color4ub(255, 255, 255)
			);
		}
		else if (lightComponentData->getLightType() == world::LtPoint)
		{
			primitiveRenderer->drawSolidPoint(lightPosition, 8.0f, Color4ub(255, 255, 0));
		}
		else if (lightComponentData->getLightType() == world::LtSpot)
		{
			primitiveRenderer->drawSolidPoint(lightPosition, 8.0f, Color4ub(255, 255, 0));

			Frustum spotFrustum;
			spotFrustum.buildPerspective(lightComponentData->getRadius(), 1.0f, 0.1f, lightComponentData->getRange());

			primitiveRenderer->pushWorld(transform.toMatrix44() * rotateX(deg2rad(90.0f)));
			primitiveRenderer->drawWireQuad(
				spotFrustum.corners[0],
				spotFrustum.corners[1],
				spotFrustum.corners[2],
				spotFrustum.corners[3],
				Color4ub(255, 255, 255)
			);
			primitiveRenderer->drawWireQuad(
				spotFrustum.corners[4],
				spotFrustum.corners[5],
				spotFrustum.corners[6],
				spotFrustum.corners[7],
				Color4ub(255, 255, 255)
			);
			primitiveRenderer->drawLine(
				spotFrustum.corners[0],
				spotFrustum.corners[4],
				Color4ub(255, 255, 255)
			);
			primitiveRenderer->drawLine(
				spotFrustum.corners[1],
				spotFrustum.corners[5],
				Color4ub(255, 255, 255)
			);
			primitiveRenderer->drawLine(
				spotFrustum.corners[2],
				spotFrustum.corners[6],
				Color4ub(255, 255, 255)
			);
			primitiveRenderer->drawLine(
				spotFrustum.corners[3],
				spotFrustum.corners[7],
				Color4ub(255, 255, 255)
			);
			primitiveRenderer->popWorld();
		}

		primitiveRenderer->popDepthState();

		primitiveRenderer->pushWorld(transform.toMatrix44());
		primitiveRenderer->drawWireAabb(Aabb3(Vector4(-0.25f, -0.25f, -0.25f, 1.0f), Vector4(0.25f, 0.25f, 0.25f, 1.0f)), m_colorBoundingBox);
		primitiveRenderer->popWorld();

		return;
	}

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
	m_colorBoundingBox = colors->getProperty< Color4ub >(L"BoundingBoxWire");
	m_colorBoundingBoxSel = colors->getProperty< Color4ub >(L"BoundingBoxWireSelected");
	m_colorBoundingBoxFaceSel = colors->getProperty< Color4ub >(L"BoundingBoxFaceSelected");
	m_colorSnap = colors->getProperty< Color4ub >(L"SnapPoint");
}

	}
}
