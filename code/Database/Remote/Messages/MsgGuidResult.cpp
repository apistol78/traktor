/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Remote/Messages/MsgGuidResult.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.MsgGuidResult", 0, MsgGuidResult, IMessage)

MsgGuidResult::MsgGuidResult(const Guid& value)
:	m_value(value)
{
}

void MsgGuidResult::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"value", m_value);
}

	}
}
