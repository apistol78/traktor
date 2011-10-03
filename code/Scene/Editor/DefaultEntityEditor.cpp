#include <limits>
#include "Core/Math/Const.h"
#include "Core/Settings/PropertyColor.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/Settings.h"
#include "Editor/IEditor.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/DefaultEntityEditor.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/IModifier.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"
#include "World/Entity/DirectionalLightEntity.h"
#include "World/Entity/GroupEntityData.h"
#include "World/Entity/SpatialEntityData.h"
#include "World/Entity/SpatialGroupEntityData.h"

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

bool DefaultEntityEditor::isGroup() const
{
	const world::EntityData* entityData = m_entityAdapter->getEntityData();
	return is_a< world::GroupEntityData >(entityData) || is_a< world::SpatialGroupEntityData >(entityData);
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
	
	if (world::SpatialGroupEntityData* spatialGroupEntityData = dynamic_type_cast< world::SpatialGroupEntityData* >(entityData))
	{
		if (world::SpatialEntityData* spatialChildEntityData = dynamic_type_cast< world::SpatialEntityData* >(childEntityData))
		{
			spatialGroupEntityData->addEntityData(spatialChildEntityData);
			return true;
		}
	}

	return false;
}

bool DefaultEntityEditor::removeChildEntity(EntityAdapter* childEntityAdapter) const
{
	world::EntityData* entityData = m_entityAdapter->getEntityData();
	world::EntityData* childEntityData = childEntityAdapter->getEntityData();

	if (world::GroupEntityData* groupEntityData = dynamic_type_cast< world::GroupEntityData* >(entityData))
	{
		groupEntityData->removeEntityData(childEntityData);
		return true;
	}

	if (world::SpatialGroupEntityData* spatialGroupEntityData = dynamic_type_cast< world::SpatialGroupEntityData* >(entityData))
	{
		if (world::SpatialEntityData* spatialChildEntityData = dynamic_type_cast< world::SpatialEntityData* >(childEntityData))
		{
			spatialGroupEntityData->removeEntityData(spatialChildEntityData);
			return true;
		}
	}

	return false;
}

bool DefaultEntityEditor::queryRay(const Vector4& worldRayOrigin, const Vector4& worldRayDirection, Scalar& outDistance) const
{
	if (!m_entityAdapter->isSpatial())
		return false;

	// Transform ray into object space.
	Transform worldInv = m_entityAdapter->getTransform().inverse();
	Vector4 objectRayOrigin = worldInv * worldRayOrigin;
	Vector4 objectRayDirection = worldInv * worldRayDirection;

	// Get entity bounding box; do not pick if origin of ray is within box.
	Aabb3 boundingBox = m_entityAdapter->getBoundingBox();
	if (boundingBox.empty() || boundingBox.inside(objectRayOrigin))
		return false;

	// Trace bounding box to see if ray intersect.
	Scalar distance;
	if (!boundingBox.intersectSegment(objectRayOrigin, objectRayOrigin + objectRayDirection * (outDistance - Scalar(FUZZY_EPSILON)), distance))
		return false;

	T_ASSERT (distance <= outDistance);
	return true;
}

void DefaultEntityEditor::entitySelected(bool selected)
{
}

void DefaultEntityEditor::cursorMoved(const ApplyParams& params)
{
}

void DefaultEntityEditor::beginModifier(const ApplyParams& params)
{
	T_ASSERT (!m_inModify);
	m_inModify = true;
	m_modifyTransform = m_entityAdapter->getTransform();
}

void DefaultEntityEditor::applyModifier(const ApplyParams& params)
{
	T_ASSERT (m_inModify);

	if (!m_entityAdapter->isSpatial())
		return;

	Ref< IModifier > modifier = m_context->getModifier();
	if (!modifier)
		return;

	Transform transform = m_modifyTransform;
	
	modifier->adjust(
		m_context,
		params.viewTransform,
		params.screenDelta,
		params.viewDelta,
		params.worldDelta,
		params.mouseButton,
		transform
	);

	// Save "unsnapped" transform.
	m_modifyTransform = transform;

	// Snap to grid.
	if (m_context->getSnapMode() == SceneEditorContext::SmGrid)
	{
		float spacing = m_context->getSnapSpacing();
		if (spacing > 0.0f)
		{
			Vector4 t = transform.translation();
			t.set(
				floor(t[0] / spacing + 0.5f) * spacing,
				floor(t[1] / spacing + 0.5f) * spacing,
				floor(t[2] / spacing + 0.5f) * spacing,
				1.0f
			);
			transform = Transform(t, transform.rotation());
		}
	}
	// Snap to neighbour entity.
	else if (m_context->getSnapMode() == SceneEditorContext::SmNeighbour)
	{
		AlignedVector< EntityAdapter::SnapPoint > snapPoints = m_entityAdapter->getSnapPoints();

		float minDistance = std::numeric_limits< float >::max();
		Vector4 minTranslate;

		RefArray< EntityAdapter > otherEntities;
		m_context->getEntities(otherEntities);

		for (RefArray< EntityAdapter >::const_iterator i = otherEntities.begin(); i != otherEntities.end(); ++i)
		{
			// Snap only to leafs.
			if (!(*i)->getChildren().empty())
				continue;

			// Ensure we're not snapping to ourself.
			bool sameEntity = false;
			for (EntityAdapter* check = *i; check; check = check->getParent())
			{
				if (check == m_entityAdapter)
				{
					sameEntity = true;
					break;
				}
			}
			if (sameEntity)
				continue;

			// Get snap points from other entity.
			AlignedVector< EntityAdapter::SnapPoint > otherSnapPoints = (*i)->getSnapPoints();

			// Find closest snap point.
			for (AlignedVector< EntityAdapter::SnapPoint >::const_iterator j = snapPoints.begin(); j != snapPoints.end(); ++j)
			{
				for (AlignedVector< EntityAdapter::SnapPoint >::const_iterator k = otherSnapPoints.begin(); k != otherSnapPoints.end(); ++k)
				{
					// Snap points must face each other.
					if (dot3(k->direction, j->direction) >= 0.0f)
						continue;

					// Remember closest snap point.
					float distance = (k->position - j->position).length();
					if (distance < minDistance)
					{
						minTranslate = k->position - j->position;
						minDistance = distance;
					}
				}
			}
		}
		
		if (minDistance <= 0.2f)
			transform = transform * Transform(minTranslate);
	}

	m_entityAdapter->setTransform(transform);
}

void DefaultEntityEditor::endModifier(const ApplyParams& params)
{
	T_ASSERT (m_inModify);
	m_inModify = false;
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

	if (is_a< world::DirectionalLightEntity >(m_entityAdapter->getEntity()))
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
	else
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
