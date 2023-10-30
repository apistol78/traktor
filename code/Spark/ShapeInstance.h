/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Spark/CharacterInstance.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::spark
{

class Shape;

/*! Shape instance.
 * \ingroup Spark
 */
class T_DLLCLASS ShapeInstance : public CharacterInstance
{
	T_RTTI_CLASS;

public:
	explicit ShapeInstance(Context* context, Dictionary* dictionary, CharacterInstance* parent, const Shape* shape);

	const Shape* getShape() const;

	virtual Aabb2 getBounds() const override final;

private:
	Ref< const Shape > m_shape;
};

}
