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

class Text;

/*! Static text instance.
 * \ingroup Spark
 */
class T_DLLCLASS TextInstance : public CharacterInstance
{
	T_RTTI_CLASS;

public:
	explicit TextInstance(Context* context, Dictionary* dictionary, CharacterInstance* parent, const Text* text);

	const Text* getText() const;

	virtual Aabb2 getBounds() const override final;

private:
	Ref< const Text > m_text;
};

	}
}

