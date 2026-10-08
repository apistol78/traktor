/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Scene/Editor/MarkerEntityComponentEditor.h"

#include "Render/ITexture.h"
#include "Render/PrimitiveRenderer.h"
#include "Resource/IResourceManager.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"

namespace traktor::scene
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.MarkerEntityComponentEditor", MarkerEntityComponentEditor, IComponentEditor)

MarkerEntityComponentEditor::MarkerEntityComponentEditor(SceneEditorContext* context, EntityAdapter* entityAdapter, MarkerEntityComponentData* markerComponentData)
	: m_entityAdapter(entityAdapter)
	, m_mode(markerComponentData->getMode())
	, m_offset(markerComponentData->getOffset())
	, m_size(markerComponentData->getSize())
	, m_alpha(markerComponentData->getAlpha())
{
	context->getResourceManager()->bind(markerComponentData->getTexture(), m_texture);
}

void MarkerEntityComponentEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	if (!m_texture)
		return;

	const Matrix44 viewInverse = primitiveRenderer->getView().inverse();
	const Scalar s(m_size * 0.5f);
	const Vector4 right = viewInverse.axisX() * s;
	const Vector4 up = viewInverse.axisY() * s;

	const auto drawQuad = [&](const Vector4& position) {
		primitiveRenderer->drawTextureQuad(
			position - right + up,
			Vector2(0.0f, 0.0f),
			position + right + up,
			Vector2(1.0f, 0.0f),
			position + right - up,
			Vector2(1.0f, 1.0f),
			position - right - up,
			Vector2(0.0f, 1.0f),
			Color4ub(255, 255, 255, (int32_t)(255 * m_alpha)),
			m_texture);
	};

	primitiveRenderer->pushDepthState(true, false, false);

	if (m_mode == MarkerMode::Single)
	{
		const Vector4 position = m_entityAdapter->getTransform().translation().xyz1() + Vector4(0.0f, m_offset, 0.0f, 0.0f);
		drawQuad(position);
	}
	else
	{
		AlignedVector< Vector4 > positions;
		for (const auto& child : m_entityAdapter->getChildren())
			positions.push_back(child->getTransform().translation().xyz1() + Vector4(0.0f, m_offset, 0.0f, 0.0f));

		if (m_mode == MarkerMode::ChildrenConnected)
		{
			for (int32_t i = 0; i < (int32_t)positions.size(); ++i)
			{
				const Vector4& from = positions[i];
				const Vector4& to = positions[(i + 1) % positions.size()];
				primitiveRenderer->drawLine(from, to, Color4ub(255, 255, 255, (int32_t)(255 * m_alpha)));
			}
		}

		for (const auto& position : positions)
			drawQuad(position);
	}

	primitiveRenderer->popDepthState();
}

}
