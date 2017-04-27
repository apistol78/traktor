/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Remote/Messages/DbmOpenTransaction.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmOpenTransaction", 0, DbmOpenTransaction, IMessage)

DbmOpenTransaction::DbmOpenTransaction(uint32_t handle)
:	m_handle(handle)
{
}

void DbmOpenTransaction::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"handle", m_handle);
}

	}
}
