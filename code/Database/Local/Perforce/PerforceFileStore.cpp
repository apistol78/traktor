/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <time.h>
#include "Core/Io/FileSystem.h"
#include "Core/Misc/String.h"
#include "Core/System/OS.h"
#include "Database/ConnectionString.h"
#include "Database/Types.h"
#include "Database/Local/Perforce/PerforceChangeList.h"
#include "Database/Local/Perforce/PerforceClient.h"
#include "Database/Local/Perforce/PerforceFileStore.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.PerforceFileStore", 0, PerforceFileStore, IFileStore)

bool PerforceFileStore::create(const ConnectionString& connectionString)
{
	time_t rawtime;
	struct tm* timeinfo;
	wchar_t buffer[1024];

	PerforceClientDesc desc;
	desc.m_host = OS::getInstance().getComputerName();
	desc.m_port = connectionString.get(L"p4port");
	desc.m_user = connectionString.get(L"p4user");
	desc.m_password= connectionString.get(L"p4pwd");
	desc.m_client = connectionString.get(L"p4client");
	desc.m_securityLevel = PerforceClientDesc::SlLow;

	// Create change list description.
	std::wstring changeListDescription = connectionString.get(L"p4changelist");
	if (changeListDescription.empty())
		changeListDescription = L"** CREATED **";

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	wcsftime(buffer, sizeof_array(buffer), changeListDescription.c_str(), timeinfo);
	changeListDescription = buffer;

	m_p4client = new PerforceClient(desc);

	RefArray< PerforceChangeList > changeLists;
	if (m_p4client->getChangeLists(changeLists))
	{
		for (auto changeList : changeLists)
		{
			if (compareIgnoreCase(changeList->getDescription(), changeListDescription) == 0)
			{
				m_p4changeList = changeList;
				break;
			}
		}
	}

	if (!m_p4changeList)
	{
		m_p4changeList = m_p4client->createChangeList(changeListDescription);
		if (!m_p4changeList)
			return false;
	}

	return true;
}

void PerforceFileStore::destroy()
{
	if (m_p4client && m_p4changeList)
	{
		// Revert unmodified files from changelist.
		m_p4client->revertUnmodifiedFiles(m_p4changeList);

		// In case no files has been modified we revert entire changelist.
		if (m_p4changeList->getFiles().empty())
			m_p4client->revertChangeList(m_p4changeList);
	}

	m_p4changeList = nullptr;
	m_p4client = nullptr;
}

uint32_t PerforceFileStore::flags(const Path& filePath)
{
#if defined(_WIN32)
	std::wstring localFile = filePath.normalized().getPathName();
#else
	std::wstring localFile = filePath.normalized().getPathNameNoVolume();
#endif

	uint32_t flags = IfNormal;

	PerforceAction action;
	m_p4client->isOpened(localFile, action);
	if (action == PerforceAction::AtNotOpened)
		flags |= IfReadOnly;
	else
		flags |= IfModified;

	return flags;
}

bool PerforceFileStore::add(const Path& filePath)
{
#if defined(_WIN32)
	std::wstring localFile = filePath.normalized().getPathName();
#else
	std::wstring localFile = filePath.normalized().getPathNameNoVolume();
#endif
	return m_p4client->addFile(m_p4changeList, localFile);
}

bool PerforceFileStore::remove(const Path& filePath)
{
#if defined(_WIN32)
	std::wstring localFile = filePath.normalized().getPathName();
#else
	std::wstring localFile = filePath.normalized().getPathNameNoVolume();
#endif

	PerforceAction pa;
	if (m_p4client->isOpened(localFile, pa))
	{
		if (pa == PerforceAction::AtAdd)
		{
			// File has been added; revert in change list then remove local.
			if (!m_p4client->revertFile(m_p4changeList, localFile))
				return false;

			return FileSystem::getInstance().remove(localFile);
		}
		else if (pa == PerforceAction::AtEdit)
		{
			// File has been opened for edit; revert in change list then open for delete.
			if (!m_p4client->revertFile(m_p4changeList, localFile))
				return false;

			return m_p4client->openForDelete(m_p4changeList, localFile);
		}
		else if (pa == PerforceAction::AtDelete)
		{
			// File has already been opened for delete.
			return true;
		}

		// File not opened but exist in perforce; open for delete.
		return m_p4client->openForDelete(m_p4changeList, localFile);
	}
	else
	{
		// File not in perforce; delete local file.
		return FileSystem::getInstance().remove(localFile);
	}
}

bool PerforceFileStore::edit(const Path& filePath)
{
#if defined(_WIN32)
	std::wstring localFile = filePath.normalized().getPathName();
#else
	std::wstring localFile = filePath.normalized().getPathNameNoVolume();
#endif

	PerforceAction pa;
	if (m_p4client->isOpened(localFile, pa))
	{
		if (pa == PerforceAction::AtAdd || pa == PerforceAction::AtEdit)
			return true;

		if (m_p4client->openForEdit(m_p4changeList, localFile))
			return true;
	}
	else
	{
		// File doesn't exist in P4; add local file.
		if (m_p4client->addFile(m_p4changeList, localFile))
			return true;
	}

	return false;
}

bool PerforceFileStore::rollback(const Path& filePath)
{
#if defined(_WIN32)
	std::wstring localFile = filePath.normalized().getPathName();
#else
	std::wstring localFile = filePath.normalized().getPathNameNoVolume();
#endif
	return m_p4client->revertFile(m_p4changeList, localFile);
}

bool PerforceFileStore::clean(const Path& filePath)
{
	return true;
}

	}
}
