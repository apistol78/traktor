/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Shape/Editor/Spline/SplineComponent.h"
#include "Shape/Editor/Spline/SplineEntityEditor.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SplineEntityEditor", SplineEntityEditor, scene::DefaultEntityEditor)

SplineEntityEditor::SplineEntityEditor(scene::SceneEditorContext* context, scene::EntityAdapter* entityAdapter)
:	scene::DefaultEntityEditor(context, entityAdapter)
{
}

void SplineEntityEditor::drawGuide(render::PrimitiveRenderer* primitiveRenderer) const
{
	 auto splineComponent = getEntityAdapter()->getComponent< SplineComponent >();
	 if (!splineComponent)
	 	return;

	 primitiveRenderer->pushDepthState(false, false, false);

	 const auto& path = splineComponent->getPath();
	 const auto& keys = path.keys();
	 if (keys.empty())
	 	return;

	 const float st = path.getStartTime();
	 const float et = path.getEndTime();

	 const uint32_t nsteps = (uint32_t)keys.size() * 10;
	 for (uint32_t i = 0; i < nsteps; ++i)
	 {
	 	const float t1 = st + (float)(i * (et - st)) / nsteps;
	 	const float t2 = st + (float)((i + 1) * (et - st)) / nsteps;
	 	primitiveRenderer->drawLine(
	 		path.evaluate(t1, true).position,
	 		path.evaluate(t2, true).position,
	 		getEntityAdapter()->isSelected() ? Color4ub(100, 100, 255, 220) : Color4ub(255, 255, 255, 80)
	 	);
	 }

	 primitiveRenderer->popDepthState();
}

	}
}
