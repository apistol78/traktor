/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Remote/Messages/MsgHandleArrayResult.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.MsgHandleArrayResult", 0, MsgHandleArrayResult, IMessage)

void MsgHandleArrayResult::add(uint32_t handle)
{
	m_handles.push_back(handle);
}

uint32_t MsgHandleArrayResult::count()
{
	return uint32_t(m_handles.size());
}

uint32_t MsgHandleArrayResult::get(uint32_t index) const
{
	return m_handles[index];
}

void MsgHandleArrayResult::serialize(ISerializer& s)
{
	s >> MemberStlVector< uint32_t >(L"handles", m_handles);
}

	}
}
