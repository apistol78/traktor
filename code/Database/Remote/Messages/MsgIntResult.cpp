/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Remote/Messages/MsgIntResult.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.MsgIntResult", 0, MsgIntResult, IMessage)

MsgIntResult::MsgIntResult(int32_t value)
:	m_value(value)
{
}

void MsgIntResult::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"value", m_value);
}

	}
}
