/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Database/IEvent.h"
#include "Database/Remote/Messages/DbmGetEventResult.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmGetEventResult", 0, DbmGetEventResult, IMessage)

DbmGetEventResult::DbmGetEventResult()
:	m_remote(false)
{
}

DbmGetEventResult::DbmGetEventResult(const IEvent* event, bool remote)
:	m_event(event)
,	m_remote(remote)
{
}

void DbmGetEventResult::serialize(ISerializer& s)
{
	s >> MemberRef< const IEvent >(L"event", m_event);
	s >> Member< bool >(L"remote", m_remote);
}

	}
}
