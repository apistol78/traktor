#include <limits>
#include "Editor/IEditor.h"
#include "Editor/Settings.h"
#include "Scene/Editor/DefaultEntityEditor.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/IModifier.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "World/Entity/SpatialEntityData.h"
#include "World/Entity/DirectionalLightEntity.h"
#include "Render/PrimitiveRenderer.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace scene
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.DefaultEntityEditor", DefaultEntityEditor, IEntityEditor)

DefaultEntityEditor::DefaultEntityEditor(SceneEditorContext* context)
:	m_inModify(false)
{
	updateSettings(context);
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
)
{
	T_ASSERT (entityAdapter->isSpatial());
}

void DefaultEntityEditor::beginModifier(
	SceneEditorContext* context,
	EntityAdapter* entityAdapter
)
{
	T_ASSERT (!m_inModify);
	m_inModify = true;
}

void DefaultEntityEditor::applyModifier(
	SceneEditorContext* context,
	EntityAdapter* entityAdapter,
	const Matrix44& viewTransform,
	const Vector4& screenDelta,
	const Vector4& viewDelta,
	const Vector4& worldDelta,
	int mouseButton
)
{
	T_ASSERT (m_inModify);
	T_ASSERT (entityAdapter->isSpatial());

	Ref< IModifier > modifier = context->getModifier();
	if (!modifier)
		return;

	Transform transform = entityAdapter->getTransform();
	
	modifier->adjust(
		context,
		viewTransform,
		screenDelta,
		viewDelta,
		worldDelta,
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
			entityAdapter->setTransform(transform * Transform(minTranslate));
	}
}

void DefaultEntityEditor::endModifier(
	SceneEditorContext* context,
	EntityAdapter* entityAdapter
)
{
	T_ASSERT (m_inModify);
	m_inModify = false;
}

bool DefaultEntityEditor::handleCommand(
	SceneEditorContext* context,
	EntityAdapter* entityAdapter,
	const ui::Command& command
)
{
	if (command == L"Editor.SettingsChanged")
		updateSettings(context);
	return false;
}

void DefaultEntityEditor::drawGuide(
	SceneEditorContext* context,
	render::PrimitiveRenderer* primitiveRenderer,
	EntityAdapter* entityAdapter
) const
{
	const Vector4 c_expandBoundingBox(0.001f, 0.001f, 0.001f, 0.0f);

	Transform transform = entityAdapter->getTransform();
	
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
			lightPosition - lightDirection * Scalar(0.5f),
			lightPosition + lightDirection * Scalar(0.5f),
			5.0f,
			Color(255, 255, 0)
		);
		primitiveRenderer->drawArrowHead(
			lightPosition + lightDirection * Scalar(0.5f),
			lightPosition + lightDirection * Scalar(0.7f),
			0.5f,
			Color(255, 255, 0)
		);

		primitiveRenderer->pushWorld(transform.toMatrix44());
		primitiveRenderer->drawWireAabb(Aabb(Vector4(-0.25f, -0.25f, -0.25f, 1.0f), Vector4(0.25f, 0.25f, 0.25f, 1.0f)), m_colorBoundingBox);
		primitiveRenderer->popWorld();
	}
	else
	{
		primitiveRenderer->pushWorld(transform.toMatrix44());
		if (entityAdapter->isSelected())
		{
			primitiveRenderer->drawSolidAabb(boundingBox, m_colorBoundingBoxFaceSel);
			primitiveRenderer->drawWireAabb(boundingBox, m_colorBoundingBoxSel);
		}
		else
			primitiveRenderer->drawWireAabb(boundingBox, m_colorBoundingBox);
		primitiveRenderer->popWorld();

		if (entityAdapter->isSelected() && context->getSnapEnable())
		{
			AlignedVector< EntityAdapter::SnapPoint > snapPoints = entityAdapter->getSnapPoints();
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

bool DefaultEntityEditor::getStatusText(
	SceneEditorContext* context,
	EntityAdapter* entityAdapter,
	std::wstring& outStatusText
) const
{
	return false;
}

void DefaultEntityEditor::updateSettings(SceneEditorContext* context)
{
	Ref< editor::PropertyGroup > colors = context->getEditor()->getSettings()->getProperty< editor::PropertyGroup >(L"Editor.Colors");
	m_colorBoundingBox = colors->getProperty< editor::PropertyColor >(L"BoundingBoxWire");
	m_colorBoundingBoxSel = colors->getProperty< editor::PropertyColor >(L"BoundingBoxWireSelected");
	m_colorBoundingBoxFaceSel = colors->getProperty< editor::PropertyColor >(L"BoundingBoxFaceSelected");
	m_colorSnap = colors->getProperty< editor::PropertyColor >(L"SnapPoint");
}

	}
}
