/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Flash/EditInstance.h"
#include "Flash/TextLayout.h"
#include "Flash/Debug/EditInstanceDebugInfo.h"

namespace traktor
{
	namespace flash
	{
	
T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.EditInstanceDebugInfo", 0, EditInstanceDebugInfo, InstanceDebugInfo)

EditInstanceDebugInfo::EditInstanceDebugInfo()
{
}

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
