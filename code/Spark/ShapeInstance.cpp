/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spark/Shape.h"
#include "Spark/ShapeInstance.h"

namespace traktor::spark
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.ShapeInstance", ShapeInstance, CharacterInstance)

ShapeInstance::ShapeInstance(Context* context, Dictionary* dictionary, CharacterInstance* parent, const Shape* shape)
:	CharacterInstance(context, dictionary, parent)
,	m_shape(shape)
{
}

const Shape* ShapeInstance::getShape() const
{
	return m_shape;
}

Aabb2 ShapeInstance::getBounds() const
{
	return getTransform() * m_shape->getShapeBounds();
}

}
