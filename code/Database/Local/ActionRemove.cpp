/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Database/Local/ActionRemove.h"
#include "Database/Local/Context.h"
#include "Database/Local/IFileStore.h"
#include "Database/Local/LocalInstanceMeta.h"
#include "Database/Local/PhysicalAccess.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.ActionRemove", ActionRemove, Action)

ActionRemove::ActionRemove(const Path& instancePath)
:	m_instancePath(instancePath)
{
}

bool ActionRemove::execute(Context& context)
{
	IFileStore* fileStore = context.getFileStore();
	Path instanceObjectPath = getInstanceObjectPath(m_instancePath);
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);

	Ref< LocalInstanceMeta > instanceMeta = readPhysicalObject< LocalInstanceMeta >(instanceMetaPath);
	if (!instanceMeta)
	{
		log::error << L"Action remove failed; unable to read meta object." << Endl;
		return false;
	}

	for (const auto& blob : instanceMeta->getBlobs())
	{
		Path instanceDataPath = getInstanceDataPath(m_instancePath, blob);
		if (fileStore->remove(instanceDataPath))
			m_renamedFiles.push_back(instanceDataPath.getPathName());
		else
		{
			log::error << L"Action remove failed; unable to remove \"" << instanceDataPath.getPathName() << L"\"." << Endl;
			return false;
		}
	}

	if (fileStore->remove(instanceObjectPath))
		m_renamedFiles.push_back(instanceObjectPath.getPathName());
	else
	{
		log::error << L"Action remove failed; unable to remove \"" << instanceObjectPath.getPathName() << L"\"." << Endl;
		return false;
	}

	if (fileStore->remove(instanceMetaPath))
		m_renamedFiles.push_back(instanceMetaPath.getPathName());
	else
	{
		log::error << L"Action remove failed; unable to remove \"" << instanceMetaPath.getPathName() << L"\"." << Endl;
		return false;
	}

	return true;
}

bool ActionRemove::undo(Context& context)
{
	IFileStore* fileStore = context.getFileStore();
	for (const auto& renamedFile : m_renamedFiles)
	{
		if (!fileStore->rollback(renamedFile))
			return false;
	}
	m_renamedFiles.clear();
	return true;
}

void ActionRemove::clean(Context& context)
{
	IFileStore* fileStore = context.getFileStore();
	for (const auto& renamedFile : m_renamedFiles)
		fileStore->clean(renamedFile);
}

bool ActionRemove::redundant(const Action* action) const
{
	if (const ActionRemove* actionRemove = dynamic_type_cast< const ActionRemove* >(action))
		return m_instancePath == actionRemove->m_instancePath;
	else
		return false;
}

	}
}
