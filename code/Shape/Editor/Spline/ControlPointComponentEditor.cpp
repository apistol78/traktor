/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Shape/Editor/Spline/ControlPointComponentEditor.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.ControlPointComponentEditor", ControlPointComponentEditor, scene::DefaultComponentEditor)

ControlPointComponentEditor::ControlPointComponentEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter, world::IEntityComponentData* componentData)
:	scene::DefaultComponentEditor(context, entityAdapter, componentData)
{
}

void ControlPointComponentEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	primitiveRenderer->pushDepthState(false, false, false);

	bool selected = m_entityAdapter->isSelected() || m_entityAdapter->getParent()->isSelected();

	const auto& T = m_entityAdapter->getTransform();
	primitiveRenderer->drawSolidPoint(
		T.translation(),
		4.0f,
		selected ? Color4ub(100, 100, 255, 220) : Color4ub(255, 255, 255, 100)
	);

	primitiveRenderer->popDepthState();
}

	}
}
