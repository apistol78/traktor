/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Editor/PathEntity/PathComponentEditor.h"
#include "Animation/PathEntity/PathComponent.h"
#include "Animation/PathEntity/PathComponentData.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.PathComponentEditor", PathComponentEditor, scene::DefaultComponentEditor)

PathComponentEditor::PathComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData)
:	scene::DefaultComponentEditor(context, entityAdapter, componentData)
{
}

void PathComponentEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	auto pathComponentData = checked_type_cast< const PathComponentData* >(m_componentData);
	auto pathComponent = dynamic_type_cast< const PathComponent* >(m_entityAdapter->getComponent< PathComponent >());

	if (m_context->shouldDrawGuide(L"Animation.Path"))
	{
		// Draw entity's path.
		const TransformPath& path = pathComponent->getPath();
		const AlignedVector< TransformPath::Key >& keys = path.getKeys();

		for (const auto& key : keys)
		{
			primitiveRenderer->drawWireAabb(
				key.position,
				Vector4(0.2f, 0.2f, 0.2f),
				1.0f,
				Color4ub(255, 255, 0)
			);
		}

		if (keys.size() >= 2)
		{
			// Draw linear curve.
			for (uint32_t i = 0; i < keys.size() - 1; ++i)
			{
				primitiveRenderer->drawLine(
					keys[i].position,
					keys[i + 1].position,
					Color4ub(0, 255, 0)
				);
			}

			// Draw evaluated curve.
			bool loop = (pathComponent->getTimeMode() == PathComponent::TimeMode::Loop);
			float st = keys.front().T;
			float et = keys.back().T;
			for (uint32_t i = 0; i < 40; ++i)
			{
				float t1 = st + (i * (et - st)) / 40.0f;
				float t2 = st + ((i + 1) * (et - st)) / 40.0f;
				primitiveRenderer->drawLine(
					path.evaluate(t1, loop).position,
					path.evaluate(t2, loop).position,
					Color4ub(170, 170, 255)
				);
			}
		}
	}
}

	}
}
