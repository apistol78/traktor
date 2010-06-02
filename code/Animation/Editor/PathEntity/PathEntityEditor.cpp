#include "Animation/Editor/PathEntity/PathEntityEditor.h"
#include "Animation/PathEntity/PathEntityData.h"
#include "Animation/PathEntity/PathEntity.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/IModifier.h"
#include "World/Entity/SpatialEntity.h"
#include "Render/PrimitiveRenderer.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.PathEntityEditor", PathEntityEditor, scene::DefaultEntityEditor)

PathEntityEditor::PathEntityEditor(scene::SceneEditorContext* context)
:	scene::DefaultEntityEditor(context)
,	m_time(0.0f)
{
}

void PathEntityEditor::entitySelected(
	scene::SceneEditorContext* context,
	scene::EntityAdapter* entityAdapter,
	bool selected
)
{
	Ref< PathEntityData > entityData = checked_type_cast< PathEntityData* >(entityAdapter->getEntityData());
	Ref< PathEntity > entity = checked_type_cast< PathEntity* >(entityAdapter->getEntity());

	if (selected)
	{
		m_time = 0.0f;
		entity->setTimeMode(PathEntity::TmManual);
	}
	else
	{
		entity->setTimeMode(entityData->getTimeMode());
	}
}

void PathEntityEditor::applyModifier(
	scene::SceneEditorContext* context,
	scene::EntityAdapter* entityAdapter,
	const Matrix44& viewTransform,
	const Vector4& screenDelta,
	const Vector4& viewDelta,
	const Vector4& worldDelta,
	int mouseButton
)
{
	Ref< PathEntityData > entityData = checked_type_cast< PathEntityData* >(entityAdapter->getEntityData());
	Ref< PathEntity > entity = checked_type_cast< PathEntity* >(entityAdapter->getEntity());

	Ref< scene::IModifier > modifier = context->getModifier();
	if (modifier)
	{
		TransformPath& path = entityData->getPath();
		TransformPath::Frame* frame = path.getClosestKeyFrame(m_time);
		if (frame)
		{
			// Use modifier to adjust closest key frame.
			Transform transform = frame->transform();
			modifier->adjust(
				context,
				viewTransform,
				screenDelta,
				viewDelta,
				worldDelta,
				mouseButton,
				transform
			);
			frame->position = transform.translation();
			frame->orientation = transform.rotation().toEulerAngles();

			// Update entity's path to reflect changes as we're editing the entity data's path
			// initially.
			if (entity)
				entity->setPath(path);
		}
	}
}

bool PathEntityEditor::handleCommand(
	scene::SceneEditorContext* context,
	scene::EntityAdapter* entityAdapter,
	const ui::Command& command
)
{
	Ref< PathEntityData > entityData = checked_type_cast< PathEntityData* >(entityAdapter->getEntityData());
	Ref< PathEntity > entity = checked_type_cast< PathEntity* >(entityAdapter->getEntity());

	if (command == L"Animation.Editor.StepForward")
	{
		m_time += 0.02f;
		if (entity)
			entity->setTime(m_time);
	}
	else if (command == L"Animation.Editor.StepBack")
	{
		m_time -= 0.02f;
		if (entity)
			entity->setTime(m_time);
	}
	else if (command == L"Animation.Editor.GotoPreviousKey")
		;
	else if (command == L"Animation.Editor.GotoNextKey")
		;
	else if (command == L"Animation.Editor.InsertKey")
	{
		TransformPath& path = entityData->getPath();
		TransformPath::Frame frame = path.evaluate(m_time, false);
		path.insert(m_time, frame);
		context->buildEntities();
	}
	else
		return false;

	return true;
}

void PathEntityEditor::drawGuide(
	scene::SceneEditorContext* context,
	render::PrimitiveRenderer* primitiveRenderer,
	scene::EntityAdapter* entityAdapter
) const
{
	Ref< PathEntity > pathEntity = checked_type_cast< PathEntity* >(entityAdapter->getEntity());

	// Draw entity's path.
	const TransformPath& path = pathEntity->getPath();
	const AlignedVector< TransformPath::Key >& keys = path.getKeys();

	for (AlignedVector< TransformPath::Key >::const_iterator i = keys.begin(); i != keys.end(); ++i)
	{
		primitiveRenderer->drawWireAabb(
			i->value.position,
			Vector4(0.2f, 0.2f, 0.2f),
			Color(255, 255, 0)
		);
	}

	if (keys.size() >= 2)
	{
		// Draw linear curve.
		for (uint32_t i = 0; i < keys.size() - 1; ++i)
		{
			primitiveRenderer->drawLine(
				keys[i].value.position,
				keys[i + 1].value.position,
				Color(0, 255, 0)
			);
		}

		// Draw evaluated curve.
		bool loop = (pathEntity->getTimeMode() == PathEntity::TmLoop);
		float st = keys.front().T;
		float et = keys.back().T;
		for (uint32_t i = 0; i < 40; ++i)
		{
			float t1 = st + (i * (et - st)) / 40.0f;
			float t2 = st + ((i + 1) * (et - st)) / 40.0f;
			primitiveRenderer->drawLine(
				path.evaluate(t1, loop).position,
				path.evaluate(t2, loop).position,
				Color(170, 170, 255)
			);
		}

		// Draw cursor.
		Vector4 cursor = path.evaluate(m_time, false).position;
		primitiveRenderer->drawSolidPoint(cursor, 3.0f, Color(255, 255, 255, 200));
	}

	// Draw attached entity's bounding box.
	if (world::SpatialEntity* attachedEntity = pathEntity->getEntity())
	{
		Transform transform;
		if (!attachedEntity->getTransform(transform))
			transform = Transform::identity();

		primitiveRenderer->pushWorld(transform.toMatrix44());
		primitiveRenderer->drawWireAabb(attachedEntity->getBoundingBox(), Color(255, 255, 0));
		primitiveRenderer->popWorld();
	}
}

	}
}
