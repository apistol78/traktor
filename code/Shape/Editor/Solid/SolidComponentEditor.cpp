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
#include "Shape/Editor/Solid/PrimitiveComponent.h"
#include "Shape/Editor/Solid/PrimitiveComponentData.h"
#include "Shape/Editor/Solid/SolidComponent.h"
#include "Shape/Editor/Solid/SolidComponentData.h"
#include "Shape/Editor/Solid/SolidComponentEditor.h"
#include "World/Entity.h"
#include "World/Entity/GroupComponent.h"

namespace traktor::shape
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SolidComponentEditor", SolidComponentEditor, scene::DefaultComponentEditor)

SolidComponentEditor::SolidComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData)
:	scene::DefaultComponentEditor(context, entityAdapter, componentData)
{
}

void SolidComponentEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	SolidComponent* solidComponent = m_entityAdapter->getComponent< SolidComponent >();
	if (!solidComponent)
		return;

	if (m_context->shouldDrawGuide(L"Shape.Solids"))
	{
		Winding3 winding;

		auto group = m_entityAdapter->getComponent< world::GroupComponent >();
		if (!group)
			return;

		RefArray< world::Entity > primitiveEntities;
		for (auto entity : group->getEntities())
		{
			if (entity->getComponent< PrimitiveComponent >() != nullptr)
				primitiveEntities.push_back(entity);
		}

		for (auto entity : primitiveEntities)
		{
			const PrimitiveComponent* primitiveComponent = entity->getComponent< PrimitiveComponent >();
			const model::Model* model = primitiveComponent->getModel();
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
						entity->getTransform() * winding[i],
						entity->getTransform() * winding[j],
						Color4ub(180, 180, 255, 100)
					);
				}			
			}
		}
	}
}

}
