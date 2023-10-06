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
#include "Core/Misc/String.h"
#include "Database/Types.h"
#include "Database/Local/LocalGroup.h"
#include "Database/Local/LocalInstance.h"
#include "Database/Local/LocalFileLink.h"
#include "Database/Local/Context.h"
#include "Database/Local/PhysicalAccess.h"

namespace traktor::db
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.LocalGroup", LocalGroup, IProviderGroup)

LocalGroup::LocalGroup(Context& context, const Path& groupPath, uint32_t flags)
:	m_context(context)
,	m_groupPath(groupPath)
,	m_flags(flags)
{
}

std::wstring LocalGroup::getName() const
{
	return m_groupPath.getFileNameNoExtension();
}

uint32_t LocalGroup::getFlags() const
{
	return m_flags;
}

bool LocalGroup::rename(const std::wstring& name)
{
	if (!FileSystem::getInstance().renameDirectory(m_groupPath, name))
		return false;

	m_groupPath = m_groupPath.getPathOnly() + L"/" + name;
	return true;
}

bool LocalGroup::remove()
{
	return FileSystem::getInstance().removeDirectory(m_groupPath);
}

Ref< IProviderGroup > LocalGroup::createGroup(const std::wstring& groupName)
{
	const Path newGroupPath = m_groupPath.getPathName() + L"/" + groupName;

	if (FileSystem::getInstance().exist(newGroupPath))
		return nullptr;

	if (!FileSystem::getInstance().makeDirectory(newGroupPath))
		return nullptr;

	return new LocalGroup(m_context, newGroupPath, GfNormal);
}

Ref< IProviderInstance > LocalGroup::createInstance(const std::wstring& instanceName, const Guid& instanceGuid)
{
	const Path instancePath = m_groupPath.getPathName() + L"/" + instanceName;

	Ref< LocalInstance > instance = new LocalInstance(m_context, instancePath);
	if (!instance->internalCreateNew(instanceGuid))
	{
		log::error << L"Local instance internalCreateNew failed!" << Endl;
		return nullptr;
	}

	return instance;
}

bool LocalGroup::getChildren(RefArray< IProviderGroup >& outChildGroups, RefArray< IProviderInstance >& outChildInstances)
{
	T_ASSERT(outChildGroups.empty());
	T_ASSERT(outChildInstances.empty());

	RefArray< File > groupFiles;
	if (!FileSystem::getInstance().find(m_groupPath.getPathName() + L"/*.*", groupFiles))
		return false;

	outChildGroups.reserve(groupFiles.size());
	outChildInstances.reserve(groupFiles.size());

	for (auto groupFile : groupFiles)
	{
		const Path& path = groupFile->getPath();

		if (groupFile->isDirectory() && path.getFileName() != L"." && path.getFileName() != L"..")
		{
			outChildGroups.push_back(new LocalGroup(
				m_context,
				path,
				GfNormal
			));
		}
		else if (!groupFile->isDirectory())
		{
			if (compareIgnoreCase(path.getExtension(), L"xdm") == 0)
			{
				outChildInstances.push_back(new LocalInstance(
					m_context,
					path.getPathNameNoExtension()
				));
			}
			else if (compareIgnoreCase(path.getExtension(), L"xgl") == 0)
			{
				Ref< LocalFileLink > link = readPhysicalObject< LocalFileLink >(path);
				if (link)
				{
					outChildGroups.push_back(new LocalGroup(
						m_context,
						Path(link->getPath()),
						GfLink
					));
				}
			}
			else if (compareIgnoreCase(path.getExtension(), L"xil") == 0)
			{
				Ref< LocalFileLink > link = readPhysicalObject< LocalFileLink >(path);
				if (link)
				{
					outChildInstances.push_back(new LocalInstance(
						m_context,
						Path(link->getPath()).getPathNameNoExtension()
					));
				}
			}
		}
	}

	return true;
}

}
