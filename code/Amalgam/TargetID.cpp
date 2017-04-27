/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/TargetID.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.TargetID", 0, TargetID, ISerializable)

TargetID::TargetID()
{
}

TargetID::TargetID(const Guid& id, const std::wstring& name)
:	m_id(id)
,	m_name(name)
{
}

const Guid& TargetID::getId() const
{
	return m_id;
}

const std::wstring& TargetID::getName() const
{
	return m_name;
}

void TargetID::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"id", m_id);
	s >> Member< std::wstring >(L"name", m_name);
}

	}
}
