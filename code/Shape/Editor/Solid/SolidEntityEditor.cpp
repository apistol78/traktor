/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Math/Winding3.h"
#include "Model/Model.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Shape/Editor/Solid/PrimitiveEntity.h"
#include "Shape/Editor/Solid/PrimitiveEntityData.h"
#include "Shape/Editor/Solid/SolidEntity.h"
#include "Shape/Editor/Solid/SolidEntityData.h"
#include "Shape/Editor/Solid/SolidEntityEditor.h"
#include "World/Entity/GroupComponent.h"

namespace traktor::shape
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SolidEntityEditor", SolidEntityEditor, scene::DefaultEntityEditor)

SolidEntityEditor::SolidEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter)
:	scene::DefaultEntityEditor(context, entityAdapter)
{
}

bool SolidEntityEditor::isPickable() const
{
	return false;
}

void SolidEntityEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer, const ui::Size& clientSize) const
{
	SolidEntity* solidEntity = dynamic_type_cast< SolidEntity* >(getEntityAdapter()->getEntity());
	if (!solidEntity)
		return;

	if (getContext()->shouldDrawGuide(L"Shape.Solids"))
	{
		Winding3 winding;

		auto group = solidEntity->getComponent< world::GroupComponent >();
		if (!group)
			return;

		RefArray< PrimitiveEntity > primitiveEntities;
		for (auto entity : group->getEntities())
		{
			if (is_a< PrimitiveEntity >(entity))
				primitiveEntities.push_back(checked_type_cast< PrimitiveEntity* >(entity));
		}

		for (auto primitiveEntity : primitiveEntities)
		{
			const model::Model* model = primitiveEntity->getModel();
			if (!model)
				continue;

			const auto& vertices = model->getVertices();
			const auto& positions = model->getPositions();

			for (const auto& polygon : model->getPolygons())
			{
				winding.clear();
				for (uint32_t i = 0; i < polygon.getVertexCount(); ++i)
				{
					const auto& vertex = vertices[polygon.getVertex(i)];
					const auto& position = positions[vertex.getPosition()];
					winding.push(position);
				}
				for (uint32_t i = 0; i < winding.size(); ++i)
				{
					const uint32_t j = (i + 1) % winding.size();
					primitiveRenderer->drawLine(
						primitiveEntity->getTransform() * winding[i],
						primitiveEntity->getTransform() * winding[j],
						Color4ub(180, 180, 255, 100)
					);
				}			
			}
		}
	}
}

}
