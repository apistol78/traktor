#include <limits>
#include "Scene/Editor/DefaultEntityEditor.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/IModifier.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "World/Entity/SpatialEntityData.h"
#include "World/Entity/DirectionalLightEntity.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.DefaultEntityEditor", DefaultEntityEditor, IEntityEditor)

TypeSet DefaultEntityEditor::getEntityTypes() const
{
	TypeSet typeSet;
	typeSet.insert(&type_of< world::SpatialEntityData >());
	return typeSet;
}

bool DefaultEntityEditor::isPickable(
	EntityAdapter* entityAdapter
) const
{
	T_ASSERT (entityAdapter->isSpatial());
	return true;
}

void DefaultEntityEditor::entitySelected(
	SceneEditorContext* context,
	EntityAdapter* entityAdapter,
	bool selected
) const
{
	T_ASSERT (entityAdapter->isSpatial());
}

void DefaultEntityEditor::applyModifier(
	SceneEditorContext* context,
	EntityAdapter* entityAdapter,
	const Matrix44& viewTransform,
	const Vector2& mouseDelta,
	int mouseButton
) const
{
	T_ASSERT (entityAdapter->isSpatial());

	Ref< IModifier > modifier = context->getModifier();
	if (!modifier)
		return;

	Matrix44 transform = entityAdapter->getTransform();
	
	modifier->adjust(
		context,
		viewTransform,
		mouseDelta * context->getDeltaScale(),
		mouseButton,
		transform
	);

	entityAdapter->setTransform(transform);

	// \hack Snap entity.
	if (context->getSnapEnable())
	{
		AlignedVector< EntityAdapter::SnapPoint > snapPoints = entityAdapter->getSnapPoints();

		float minDistance = std::numeric_limits< float >::max();
		Vector4 minTranslate;

		RefArray< EntityAdapter > otherEntities;
		context->getEntities(otherEntities);

		for (RefArray< EntityAdapter >::const_iterator i = otherEntities.begin(); i != otherEntities.end(); ++i)
		{
			// Snap only to leafs.
			if (!(*i)->getChildren().empty())
				continue;

			// Ensure we're not snapping to ourself.
			bool sameEntity = false;
			for (EntityAdapter* check = *i; check; check = check->getParent())
			{
				if (check == entityAdapter)
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
			entityAdapter->setTransform(transform * translate(minTranslate));
	}
}

bool DefaultEntityEditor::handleCommand(
	SceneEditorContext* context,
	EntityAdapter* entityAdapter,
	const ui::Command& command
) const
{
	return false;
}

void DefaultEntityEditor::drawGuide(
	SceneEditorContext* context,
	render::PrimitiveRenderer* primitiveRenderer,
	EntityAdapter* entityAdapter
) const
{
	const Vector4 c_expandBoundingBox(0.001f, 0.001f, 0.001f, 0.0f);

	if (!context->getGuideEnable() && !entityAdapter->isSelected())
		return;

	Matrix44 transform = entityAdapter->getTransform();
	
	Aabb boundingBox = entityAdapter->getBoundingBox();
	boundingBox.mn -= c_expandBoundingBox;
	boundingBox.mx += c_expandBoundingBox;

	if (is_a< world::DirectionalLightEntity >(entityAdapter->getEntity()))
	{
		Vector4 lightPosition = transform.translation();
		Vector4 lightDirection = -transform.axisY();
		Vector4 lightX = transform.axisX();
		Vector4 lightZ = transform.axisZ();

		primitiveRenderer->drawLine(
			lightPosition - lightDirection,
			lightPosition + lightDirection,
			5.0f,
			Color(255, 255, 0)
		);
		primitiveRenderer->drawLine(
			lightPosition + lightDirection * Scalar(0.75f) + lightX * Scalar(0.25f),
			lightPosition + lightDirection,
			5.0f,
			Color(255, 255, 0)
		);
		primitiveRenderer->drawLine(
			lightPosition + lightDirection * Scalar(0.75f) - lightX * Scalar(0.25f),
			lightPosition + lightDirection,
			5.0f,
			Color(255, 255, 0)
		);
		primitiveRenderer->drawLine(
			lightPosition + lightDirection * Scalar(0.75f) + lightZ * Scalar(0.25f),
			lightPosition + lightDirection,
			5.0f,
			Color(255, 255, 0)
		);
		primitiveRenderer->drawLine(
			lightPosition + lightDirection * Scalar(0.75f) - lightZ * Scalar(0.25f),
			lightPosition + lightDirection,
			5.0f,
			Color(255, 255, 0)
		);

		primitiveRenderer->pushWorld(transform);
		primitiveRenderer->drawWireAabb(Aabb(Vector4(-0.25f, -0.25f, -0.25f, 1.0f), Vector4(0.25f, 0.25f, 0.25f, 1.0f)), Color(255, 255, 0));
		primitiveRenderer->popWorld();
	}
	else
	{
		primitiveRenderer->pushWorld(transform);
		if (entityAdapter->isSelected())
		{
			primitiveRenderer->drawSolidAabb(boundingBox, Color(255, 128, 128, 128));
			primitiveRenderer->drawWireAabb(boundingBox, Color(255, 255, 0));
		}
		else
			primitiveRenderer->drawWireAabb(boundingBox, Color(255, 255, 0, 200));
		primitiveRenderer->popWorld();

		if (entityAdapter->isSelected() && context->getSnapEnable())
		{
			Matrix44 viewInverse = primitiveRenderer->getView().inverseOrtho();

			Scalar cameraDistance = (transform * primitiveRenderer->getView()).translation().length();
			Scalar snapSize = cameraDistance * Scalar(0.005f);

			Vector4 axisX = viewInverse.axisX() * snapSize;
			Vector4 axisY = viewInverse.axisY() * snapSize;

			AlignedVector< EntityAdapter::SnapPoint > snapPoints = entityAdapter->getSnapPoints();
			for (AlignedVector< EntityAdapter::SnapPoint >::const_iterator i = snapPoints.begin(); i != snapPoints.end(); ++i)
			{
				primitiveRenderer->drawSolidQuad(
					i->position - axisX + axisY,
					i->position + axisX + axisY,
					i->position + axisX - axisY,
					i->position - axisX - axisY,
					Color(128, 0, 255)
				);
			}
		}
	}
}

bool DefaultEntityEditor::getStatusText(
	SceneEditorContext* context,
	EntityAdapter* entityAdapter,
	std::wstring& outStatusText
) const
{
	return false;
}

	}
}
