/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spark/ButtonInstance.h"
#include "Spark/Debug/ButtonInstanceDebugInfo.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.ButtonInstanceDebugInfo", 0, ButtonInstanceDebugInfo, InstanceDebugInfo)

ButtonInstanceDebugInfo::ButtonInstanceDebugInfo()
{
}

ButtonInstanceDebugInfo::ButtonInstanceDebugInfo(const ButtonInstance* instance)
{
	m_name = instance->getName();
	m_bounds = instance->getLocalBounds();
	m_localTransform = instance->getTransform();
	m_globalTransform = instance->getFullTransform();
	m_cxform = instance->getFullColorTransform();
	m_visible = instance->isVisible();
}

void ButtonInstanceDebugInfo::serialize(ISerializer& s)
{
	InstanceDebugInfo::serialize(s);
}

	}
}
