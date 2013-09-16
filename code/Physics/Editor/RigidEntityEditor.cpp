#include "Core/Math/Const.h"
#include "Physics/Body.h"
#include "Physics/BodyDesc.h"
#include "Physics/Editor/RigidEntityEditor.h"
#include "Physics/World/RigidEntity.h"
#include "Physics/World/RigidEntityData.h"
#include "Render/PrimitiveRenderer.h"
#include "Resource/IResourceManager.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"
#include "World/Entity.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.RigidEntityEditor", RigidEntityEditor, scene::DefaultEntityEditor)

RigidEntityEditor::RigidEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter)
:	scene::DefaultEntityEditor(context, entityAdapter)
,	m_showHull(false)
{
}

void RigidEntityEditor::entitySelected(bool selected)
{
	if (selected)
		m_showHull = false;
}

bool RigidEntityEditor::handleCommand(const ui::Command& command)
{
	if (command == L"Physics.ToggleMeshTriangles")
	{
		m_showHull = !m_showHull;
		return true;
	}
	else
		return false;
}

void RigidEntityEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	Ref< RigidEntityData > rigidEntityData = checked_type_cast< RigidEntityData* >(getEntityAdapter()->getEntityData());
	Ref< RigidEntity > rigidEntity = dynamic_type_cast< RigidEntity* >(getEntityAdapter()->getEntity());

	if (getContext()->shouldDrawGuide(L"Physics.Shapes"))
	{
		const BodyDesc* bodyDesc = rigidEntityData->getBodyDesc();
		if (bodyDesc)
		{
			const ShapeDesc* shapeDesc = bodyDesc->getShape();
			if (shapeDesc)
			{
				Transform body1Transform0 = getEntityAdapter()->getTransform0();
				Transform body1Transform = getEntityAdapter()->getTransform();

				m_physicsRenderer.draw(
					getContext()->getResourceManager(),
					primitiveRenderer,
					body1Transform0,
					body1Transform,
					shapeDesc
				);
			}
		}

		if (rigidEntity)
		{
			Body* body = rigidEntity->getBody();
			if (body)
			{
				Transform bodyTransform = body->getCenterTransform();
				primitiveRenderer->pushWorld(bodyTransform.toMatrix44());
				primitiveRenderer->drawSolidPoint(Vector4::origo(), 8.0f, Color4ub(255, 255, 0, 255));
				primitiveRenderer->popWorld();
			}
		}
	}
}

	}
}
