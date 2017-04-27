/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Remote/Messages/MsgGetChildrenResult.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.MsgGetChildrenResult", 0, MsgGetChildrenResult, IMessage)

void MsgGetChildrenResult::addGroup(uint32_t handle)
{
	m_groupHandles.push_back(handle);
}

void MsgGetChildrenResult::addInstance(uint32_t handle)
{
	m_instanceHandles.push_back(handle);
}

const std::vector< uint32_t >& MsgGetChildrenResult::getGroups() const
{
	return m_groupHandles;
}

const std::vector< uint32_t >& MsgGetChildrenResult::getInstances() const
{
	return m_instanceHandles;
}

void MsgGetChildrenResult::serialize(ISerializer& s)
{
	s >> MemberStlVector< uint32_t >(L"groupHandles", m_groupHandles);
	s >> MemberStlVector< uint32_t >(L"instanceHandles", m_instanceHandles);
}

	}
}
