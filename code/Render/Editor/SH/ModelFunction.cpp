/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Math/SahTree.h"
#include "Render/Editor/SH/ModelFunction.h"

namespace traktor::render
{

T_IMPLEMENT_RTTI_CLASS(L"ModelFunction", ModelFunction, render::SHFunction)

ModelFunction::ModelFunction(model::Model* model)
:	m_model(model)
{
	m_sahTree = new SahTree();

	// const AlignedVector< model::Polygon >& polygons = m_model->getPolygons();
	// for (AlignedVector< model::Polygon >::const_iterator i = polygons.begin(); i != polygons.end(); ++i)
	// {
	// 	if (i->getVertices().size() != 3)
	// 		continue;

	// 	m_sahTree->addTriangle(
	// 		m_model->getPosition(m_model->getVertex(i->getVertex(0)).getPosition()),
	// 		m_model->getPosition(m_model->getVertex(i->getVertex(1)).getPosition()),
	// 		m_model->getPosition(m_model->getVertex(i->getVertex(2)).getPosition())
	// 	);
	// }

	// m_sahTree->build();
}

void ModelFunction::setEvaluationPoint(const Vector4& position, const Vector4& normal)
{
	m_position = position;
	m_normal = normal;
}

Vector4 ModelFunction::evaluate(float phi, float theta, const Vector4& unit) const
{
	return Vector4::zero();
}

}
