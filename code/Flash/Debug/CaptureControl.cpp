/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberEnum.h"
#include "Flash/Debug/CaptureControl.h"

namespace traktor
{
	namespace flash
	{
	
T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.CaptureControl", 0, CaptureControl, ISerializable)

CaptureControl::CaptureControl()
:	m_mode(MdStop)
{
}

CaptureControl::CaptureControl(Mode mode)
:	m_mode(mode)
{
}

void CaptureControl::serialize(ISerializer& s)
{
	s >> MemberEnumByValue< Mode >(L"mode", m_mode);
}

	}
}
