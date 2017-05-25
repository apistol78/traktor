/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Flash/Debug/CaptureControl.h"

namespace traktor
{
	namespace flash
	{
	
T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.CaptureControl", 0, CaptureControl, ISerializable)

CaptureControl::CaptureControl()
:	m_frameCount(0)
{
}

CaptureControl::CaptureControl(int32_t frameCount)
:	m_frameCount(frameCount)
{
}

void CaptureControl::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"frameCount", m_frameCount);
}

	}
}
