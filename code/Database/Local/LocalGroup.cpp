#include "Database/Local/LocalGroup.h"
#include "Database/Local/LocalInstance.h"
#include "Database/Local/LocalFileLink.h"
#include "Database/Local/Context.h"
#include "Database/Local/PhysicalAccess.h"
#include "Core/Io/FileSystem.h"
#include "Core/Misc/String.h"

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
		return false;

	if (!FileSystem::getInstance().makeDirectory(newGroupPath))
		return false;

	return gc_new< LocalGroup >(m_context, newGroupPath);
}

Ref< IProviderInstance > LocalGroup::createInstance(const std::wstring& instanceName, const Guid& instanceGuid)
{
	Path instancePath = m_groupPath.getPathName() + L"/" + instanceName;

	Ref< LocalInstance > instance = gc_new< LocalInstance >(m_context);
	if (!instance->internalCreateNew(instancePath, instanceGuid))
		return 0;

	return instance;
}

bool LocalGroup::getChildGroups(RefArray< IProviderGroup >& outChildGroups)
{
	T_ASSERT (outChildGroups.empty());

	RefArray< File > groupFiles;
	if (!FileSystem::getInstance().find(m_groupPath.getPathName() + L"/*.*", groupFiles))
		return false;

	for (RefArray< File >::iterator i = groupFiles.begin(); i != groupFiles.end(); ++i)
	{
		const Path& path = (*i)->getPath();
		if ((*i)->isDirectory() && path.getFileName() != L"." && path.getFileName() != L"..")
		{
			outChildGroups.push_back(gc_new< LocalGroup >(
				m_context,
				cref(path)
			));
		}
		else if (!(*i)->isDirectory() && compareIgnoreCase(path.getExtension(), L"xgl") == 0)
		{
			Ref< LocalFileLink > link = readPhysicalObject< LocalFileLink >(path);
			if (link)
			{
				outChildGroups.push_back(gc_new< LocalGroup >(
					m_context,
					cref(Path(link->getPath()))
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

	for (RefArray< File >::iterator i = groupFiles.begin(); i != groupFiles.end(); ++i)
	{
		const Path& path = (*i)->getPath();
		if (compareIgnoreCase(path.getExtension(), L"xdm") == 0)
		{
			Ref< LocalInstance > instance = gc_new< LocalInstance >(m_context);
			if (instance->internalCreate(path.getPathNameNoExtension()))
				outChildInstances.push_back(instance);
		}
		else if (compareIgnoreCase(path.getExtension(), L"xil") == 0)
		{
			Ref< LocalFileLink > link = readPhysicalObject< LocalFileLink >(path);
			if (link)
			{
				Ref< LocalInstance > instance = gc_new< LocalInstance >(m_context);
				if (instance->internalCreate(Path(link->getPath()).getPathNameNoExtension()))
					outChildInstances.push_back(instance);
			}
		}
	}

	return true;
}

	}
}
