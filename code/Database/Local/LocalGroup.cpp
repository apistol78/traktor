#include "Database/Local/LocalGroup.h"
#include "Database/Local/LocalInstance.h"
#include "Database/Local/LocalContext.h"
#include "Database/Local/DataAccess.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.LocalGroup", LocalGroup, IProviderGroup)

LocalGroup::LocalGroup(LocalContext* context, const Path& groupPath)
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
	if (!m_context->getDataAccess()->renameGroup(m_groupPath, name))
		return false;

	m_groupPath = m_groupPath.getPathOnly() + L"/" + name;
	return true;
}

bool LocalGroup::remove()
{
	return m_context->getDataAccess()->removeGroup(m_groupPath);
}

IProviderGroup* LocalGroup::createGroup(const std::wstring& groupName)
{
	if (!m_context->getDataAccess()->createGroup(m_groupPath, groupName))
		return 0;

	Path groupPath = std::wstring(m_groupPath) + L"/" + groupName;

	return gc_new< LocalGroup >(m_context, groupPath);
}

IProviderInstance* LocalGroup::createInstance(const std::wstring& instanceName, const Guid& instanceGuid)
{
	Path instancePath = std::wstring(m_groupPath) + L"/" + instanceName;

	Ref< LocalInstance > instance = gc_new< LocalInstance >(m_context);
	if (!instance->internalCreateNew(instancePath, instanceGuid))
		return 0;

	return instance;
}

bool LocalGroup::getChildGroups(RefArray< IProviderGroup >& outChildGroups)
{
	T_ASSERT (outChildGroups.empty());

	std::vector< Path > groupPaths;
	if (!m_context->getDataAccess()->enumerateGroups(m_groupPath, groupPaths))
		return false;

	for (std::vector< Path >::iterator i = groupPaths.begin(); i != groupPaths.end(); ++i)
		outChildGroups.push_back(gc_new< LocalGroup >(m_context, *i));

	return true;
}

bool LocalGroup::getChildInstances(RefArray< IProviderInstance >& outChildInstances)
{
	T_ASSERT (outChildInstances.empty());

	std::vector< Path > instancePaths;
	if (!m_context->getDataAccess()->enumerateInstances(m_groupPath, instancePaths))
		return false;

	for (std::vector< Path >::iterator i = instancePaths.begin(); i != instancePaths.end(); ++i)
	{
		Ref< LocalInstance > instance = gc_new< LocalInstance >(m_context);
		if (instance->internalCreateExisting(i->getPathNameNoExtension()))
			outChildInstances.push_back(instance);
	}

	return true;
}

	}
}
