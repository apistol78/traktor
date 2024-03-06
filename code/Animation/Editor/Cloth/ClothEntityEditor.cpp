/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Cloth/ClothComponent.h"
#include "Animation/Editor/Cloth/ClothEntityEditor.h"
#include "Core/Math/Const.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Ui/Command.h"

namespace traktor::animation
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
		Ref< ClothComponent > clothComponent = getEntityAdapter()->getComponent< ClothComponent >();
		clothComponent->reset();
		return true;
	}
	return scene::DefaultEntityEditor::handleCommand(command);
}

void ClothEntityEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer, const ui::Size& clientSize) const
{
	if (getContext()->shouldDrawGuide(L"Animation.Cloth"))
	{
		Ref< ClothComponent > clothComponent = getEntityAdapter()->getComponent< ClothComponent >();

		const Transform transform = getEntityAdapter()->getTransform();
		const Aabb3 boundingBox = getEntityAdapter()->getBoundingBox();
		
		primitiveRenderer->pushWorld(transform.toMatrix44());
		primitiveRenderer->drawWireAabb(boundingBox, 1.0f, Color4ub(255, 255, 0, 200));

		if (getEntityAdapter()->isSelected())
		{
			const auto& nodes = clothComponent->getNodes();

			for (const auto& edge : clothComponent->getEdges())
			{
				primitiveRenderer->drawLine(
					nodes[edge.index[0]].position[0],
					nodes[edge.index[1]].position[0],
					Color4ub(255, 255, 255)
				);
			}

			for (const auto& node : nodes)
			{
				if (node.invMass <= Scalar(FUZZY_EPSILON))
				{
					primitiveRenderer->drawSolidAabb(
						node.position[0],
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
