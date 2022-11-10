/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spark/TextInstance.h"
#include "Spark/Text.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.TextInstance", TextInstance, CharacterInstance)

TextInstance::TextInstance(Context* context, Dictionary* dictionary, CharacterInstance* parent, const Text* text)
:	CharacterInstance(context, dictionary, parent)
,	m_text(text)
{
}

const Text* TextInstance::getText() const
{
	return m_text;
}

Aabb2 TextInstance::getBounds() const
{
	return getTransform() * m_text->getTextBounds();
}

	}
}
