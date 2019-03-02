#include "Animation/Cloth/ClothEntity.h"
#include "Animation/Editor/Cloth/ClothEntityEditor.h"
#include "Core/Math/Const.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.ClothEntityEditor", ClothEntityEditor, scene::DefaultEntityEditor)

ClothEntityEditor::ClothEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter)
:	scene::DefaultEntityEditor(context, entityAdapter)
{
}

bool ClothEntityEditor::handleCommand(const ui::Command& command)
{
	if (command == L"Animation.Editor.Reset")
	{
		Ref< ClothEntity > clothEntity = checked_type_cast< ClothEntity* >(getEntityAdapter()->getEntity());
		clothEntity->reset();
		return true;
	}
	return scene::DefaultEntityEditor::handleCommand(command);
}

void ClothEntityEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	if (getContext()->shouldDrawGuide(L"Animation.Cloth"))
	{
		Ref< ClothEntity > clothEntity = checked_type_cast< ClothEntity* >(getEntityAdapter()->getEntity());

		Aabb3 boundingBox = clothEntity->getBoundingBox();
		Transform transform;

		clothEntity->getTransform(transform);
		primitiveRenderer->pushWorld(transform.toMatrix44());
		primitiveRenderer->drawWireAabb(boundingBox, Color4ub(255, 255, 0, 200));

		if (getEntityAdapter()->isSelected())
		{
			const AlignedVector< ClothEntity::Node >& nodes = clothEntity->getNodes();
			const AlignedVector< ClothEntity::Edge >& edges = clothEntity->getEdges();

			for (AlignedVector< ClothEntity::Edge >::const_iterator i = edges.begin(); i != edges.end(); ++i)
			{
				primitiveRenderer->drawLine(
					nodes[i->index[0]].position[0],
					nodes[i->index[1]].position[0],
					Color4ub(255, 255, 255)
				);
			}

			for (AlignedVector< ClothEntity::Node >::const_iterator i = nodes.begin(); i != nodes.end(); ++i)
			{
				if (i->invMass <= Scalar(FUZZY_EPSILON))
				{
					primitiveRenderer->drawSolidAabb(
						i->position[0],
						Vector4(0.1f, 0.05f, 0.05f, 0.05f),
						Color4ub(255, 0, 255, 128)
					);
				}
			}
		}

		primitiveRenderer->popWorld();
	}
}

	}
}
