/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Spark/Shape.h"
#include "Spark/ShapeInstance.h"
#include "Spark/Debug/ShapeInstanceDebugInfo.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.ShapeInstanceDebugInfo", 0, ShapeInstanceDebugInfo, InstanceDebugInfo)

ShapeInstanceDebugInfo::ShapeInstanceDebugInfo(const ShapeInstance* instance, bool mask, bool clipped)
{
	m_name = instance->getName();
	m_bounds = instance->getBounds();
	m_localTransform = instance->getTransform();
	m_globalTransform = instance->getFullTransform();
	m_cxform = instance->getFullColorTransform();
	m_visible = instance->isVisible();
	m_shape = instance->getShape();
	m_mask = mask;
	m_clipped = clipped;
}

void ShapeInstanceDebugInfo::serialize(ISerializer& s)
{
	InstanceDebugInfo::serialize(s);

	s >> MemberRef< const Shape >(L"shape", m_shape);
	s >> Member< bool >(L"mask", m_mask);
	s >> Member< bool >(L"clipped", m_clipped);
}

	}
}
