#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Database/Local/LocalGroup.h"
#include "Database/Local/LocalInstance.h"
#include "Database/Local/LocalFileLink.h"
#include "Database/Local/Context.h"
#include "Database/Local/PhysicalAccess.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.LocalGroup", LocalGroup, IProviderGroup)

LocalGroup::LocalGroup(Context* context, const Path& groupPath)
:	m_context(context)
,	m_groupPath(groupPath)
{
}

std::wstring LocalGroup::getName() const
{
	return m_groupPath.getFileNameNoExtension();
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
	Path newGroupPath = m_groupPath.getPathName() + L"/" + groupName;

	if (FileSystem::getInstance().exist(newGroupPath))
	{
		log::error << L"GROUP ALREADY EXIST" << Endl;
		return 0;
	}

	if (!FileSystem::getInstance().makeDirectory(newGroupPath))
	{
		log::error << L"UNABLE TO CREATE PHYSICAL GROUP " << newGroupPath.getPathName() << Endl;
		return 0;
	}

	return new LocalGroup(m_context, newGroupPath);
}

Ref< IProviderInstance > LocalGroup::createInstance(const std::wstring& instanceName, const Guid& instanceGuid)
{
	Path instancePath = m_groupPath.getPathName() + L"/" + instanceName;

	Ref< LocalInstance > instance = new LocalInstance(m_context, instancePath);
	if (!instance->internalCreateNew(instanceGuid))
	{
		log::error << L"Local instance internalCreateNew failed!" << Endl;
		return 0;
	}

	return instance;
}

bool LocalGroup::getChildGroups(RefArray< IProviderGroup >& outChildGroups)
{
	T_ASSERT (outChildGroups.empty());

	RefArray< File > groupFiles;
	if (!FileSystem::getInstance().find(m_groupPath.getPathName() + L"/*.*", groupFiles))
		return false;

	outChildGroups.reserve(groupFiles.size());
	for (RefArray< File >::iterator i = groupFiles.begin(); i != groupFiles.end(); ++i)
	{
		const Path& path = (*i)->getPath();
		if ((*i)->isDirectory() && path.getFileName() != L"." && path.getFileName() != L"..")
		{
			outChildGroups.push_back(new LocalGroup(
				m_context,
				path
			));
		}
		else if (!(*i)->isDirectory() && compareIgnoreCase< std::wstring >(path.getExtension(), L"xgl") == 0)
		{
			Ref< LocalFileLink > link = readPhysicalObject< LocalFileLink >(path);
			if (link)
			{
				outChildGroups.push_back(new LocalGroup(
					m_context,
					Path(link->getPath())
				));
			}
		}
	}

	return true;
}

bool LocalGroup::getChildInstances(RefArray< IProviderInstance >& outChildInstances)
{
	T_ASSERT (outChildInstances.empty());

	RefArray< File > groupFiles;
	if (!FileSystem::getInstance().find(m_groupPath.getPathName() + L"/*.*", groupFiles))
		return false;

	outChildInstances.reserve(groupFiles.size());
	for (RefArray< File >::iterator i = groupFiles.begin(); i != groupFiles.end(); ++i)
	{
		const Path& path = (*i)->getPath();
		if (compareIgnoreCase< std::wstring >(path.getExtension(), L"xdm") == 0)
		{
			outChildInstances.push_back(new LocalInstance(
				m_context,
				path.getPathNameNoExtension()
			));
		}
		else if (compareIgnoreCase< std::wstring >(path.getExtension(), L"xil") == 0)
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

	return true;
}

	}
}
