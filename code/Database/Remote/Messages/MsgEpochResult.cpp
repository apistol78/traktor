/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Remote/Messages/MsgEpochResult.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.MsgEpochResult", 0, MsgEpochResult, IMessage)

MsgEpochResult::MsgEpochResult(uint64_t value)
:	m_value(value)
{
}

void MsgEpochResult::serialize(ISerializer& s)
{
	s >> Member< uint64_t >(L"value", m_value);
}

	}
}
