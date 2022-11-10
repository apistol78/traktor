/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spark/MorphShape.h"
#include "Spark/MorphShapeInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.MorphShapeInstance", MorphShapeInstance, CharacterInstance)

MorphShapeInstance::MorphShapeInstance(Context* context, Dictionary* dictionary, CharacterInstance* parent, const MorphShape* shape)
:	CharacterInstance(context, dictionary, parent)
,	m_shape(shape)
{
}

const MorphShape* MorphShapeInstance::getShape() const
{
	return m_shape;
}

Aabb2 MorphShapeInstance::getBounds() const
{
	return getTransform() * m_shape->getShapeBounds();
}

	}
}
