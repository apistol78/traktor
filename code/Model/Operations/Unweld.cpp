/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Model/Model.h"
#include "Model/Operations/Unweld.h"

namespace traktor::model
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.Unweld", Unweld, IModelOperation)

bool Unweld::apply(Model& model) const
{
	AlignedVector< Vertex > inputVertices = model.getVertices();

	model.setVertices(AlignedVector< Vertex >());
	for (auto& polygon : model.getPolygons())
	{
		auto polverts = polygon.getVertices();
		for (auto& polvert : polverts)
			polvert = model.addVertex(inputVertices[polvert]);
		polygon.setVertices(polverts);
	}

	return true;
}

}
