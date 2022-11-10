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

namespace traktor
{
	namespace spark
	{

class MorphShape;

/*! Morph shape instance.
 * \ingroup Spark
 */
class T_DLLCLASS MorphShapeInstance : public CharacterInstance
{
	T_RTTI_CLASS;

public:
	explicit MorphShapeInstance(Context* context, Dictionary* dictionary, CharacterInstance* parent, const MorphShape* shape);

	const MorphShape* getShape() const;

	virtual Aabb2 getBounds() const override final;

private:
	Ref< const MorphShape > m_shape;
};

	}
}

