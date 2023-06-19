/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Spark/ICharacterFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPARK_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Matrix33;

}

namespace traktor::spark
{

/*!
 * \ingroup Spark
 */
class T_DLLCLASS DefaultCharacterFactory : public ICharacterFactory
{
	T_RTTI_CLASS;

public:
	virtual Ref< CharacterInstance > createInstance(
		const Character* character,
		int32_t depth,
		Context* context,
		Dictionary* dictionary,
		CharacterInstance* parent,
		const std::string& name,
		const Matrix33& transform
	) const override final;

	virtual void removeInstance(CharacterInstance* instance, int32_t depth) const override final;
};

}
