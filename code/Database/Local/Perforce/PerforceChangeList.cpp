/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Database/Local/Perforce/PerforceChangeList.h"
#include "Database/Local/Perforce/PerforceChangeListFile.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.PerforceChangeList", 0, PerforceChangeList, ISerializable)

PerforceChangeList::PerforceChangeList()
:	m_change(0)
{
}

PerforceChangeList::PerforceChangeList(
	uint32_t change,
	const std::wstring& user,
	const std::wstring& client,
	const std::wstring& description
)
:	m_change(change)
,	m_user(user)
,	m_client(client)
,	m_description(description)
{
}

uint32_t PerforceChangeList::getChange() const
{
	return m_change;
}

const std::wstring& PerforceChangeList::getUser() const
{
	return m_user;
}

const std::wstring& PerforceChangeList::getClient() const
{
	return m_client;
}

const std::wstring& PerforceChangeList::getDescription() const
{
	return m_description;
}

void PerforceChangeList::setFiles(const RefArray< PerforceChangeListFile >& files)
{
	m_files = files;
}

const RefArray< PerforceChangeListFile >& PerforceChangeList::getFiles() const
{
	return m_files;
}

void PerforceChangeList::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"change", m_change);
	s >> Member< std::wstring >(L"user", m_user);
	s >> Member< std::wstring >(L"client", m_client);
	s >> Member< std::wstring >(L"description", m_description);
	s >> MemberRefArray< PerforceChangeListFile >(L"files", m_files);
}

	}
}
