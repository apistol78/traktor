/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <limits>
#include "Core/Math/SahTree.h"
#include "Render/Editor/SH/ModelFunction.h"

namespace traktor
{
	namespace render
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

float ModelFunction::evaluate(float phi, float theta, const Vector4& unit) const
{
	Scalar diffuse = dot3(m_normal, unit);
	if (diffuse <= 0.0f)
		return 0.0f;

	T_ASSERT (diffuse >= 0.0 && diffuse <= 1.0);

	// if (m_sahTree->queryAnyIntersection(m_position + unit * 0.01f, unit, std::numeric_limits< float >::max()))
	// 	return 0.0f;

	return diffuse;
}
		
	}
}
