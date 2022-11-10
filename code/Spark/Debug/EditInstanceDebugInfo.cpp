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
#include "Core/Serialization/MemberEnum.h"
#include "Spark/EditInstance.h"
#include "Spark/TextLayout.h"
#include "Spark/Debug/EditInstanceDebugInfo.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.EditInstanceDebugInfo", 0, EditInstanceDebugInfo, InstanceDebugInfo)

EditInstanceDebugInfo::EditInstanceDebugInfo(const EditInstance* instance)
{
	m_name = instance->getName();
	m_bounds = instance->getTextBounds();
	m_localTransform = instance->getTransform();
	m_globalTransform = instance->getFullTransform();
	m_cxform = instance->getFullColorTransform();
	m_visible = instance->isVisible();
	m_text = instance->getText();
	m_textColor = instance->getTextColor();
	m_textAlign = instance->getTextLayout()->getAlignment();
	m_textSize = instance->getTextSize();
}

void EditInstanceDebugInfo::serialize(ISerializer& s)
{
	InstanceDebugInfo::serialize(s);

	s >> Member< std::wstring >(L"text", m_text);
	s >> Member< Color4f >(L"textColor", m_textColor);
	s >> MemberEnumByValue< SwfTextAlignType >(L"textAlign", m_textAlign);
	s >> Member< Vector2 >(L"textSize", m_textSize);
}

	}
}
