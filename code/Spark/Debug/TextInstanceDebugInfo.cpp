/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spark/Text.h"
#include "Spark/TextInstance.h"
#include "Spark/Debug/TextInstanceDebugInfo.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.TextInstanceDebugInfo", 0, TextInstanceDebugInfo, InstanceDebugInfo)

TextInstanceDebugInfo::TextInstanceDebugInfo(const TextInstance* instance)
{
	m_name = instance->getName();
	m_bounds = instance->getText()->getTextBounds();
	m_localTransform = instance->getTransform();
	m_globalTransform = instance->getFullTransform();
	m_cxform = instance->getFullColorTransform();
	m_visible = instance->isVisible();
}

void TextInstanceDebugInfo::serialize(ISerializer& s)
{
	InstanceDebugInfo::serialize(s);
}

	}
}
