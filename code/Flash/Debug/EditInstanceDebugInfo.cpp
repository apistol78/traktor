/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Flash/FlashEditInstance.h"
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

EditInstanceDebugInfo::EditInstanceDebugInfo(const FlashEditInstance* instance)
{
	m_name = instance->getName();
	m_bounds = instance->getTextBounds();
	m_localTransform = instance->getTransform();
	m_globalTransform = instance->getFullTransform();
	m_cxform = instance->getFullColorTransform();
	m_visible = instance->isVisible();
	m_text = instance->getText();
	m_textAlign = instance->getTextLayout()->getAlignment();
}

void EditInstanceDebugInfo::serialize(ISerializer& s)
{
	InstanceDebugInfo::serialize(s);

	s >> Member< std::wstring >(L"text", m_text);
	s >> MemberEnumByValue< SwfTextAlignType >(L"textAlign", m_textAlign);
}

	}
}
