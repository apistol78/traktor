#include "Database/Local/ActionRemove.h"
#include "Database/Local/Context.h"
#include "Database/Local/PhysicalAccess.h"
#include "Database/Local/LocalInstanceMeta.h"
#include "Core/Io/FileSystem.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.ActionRemove", ActionRemove, Action)

ActionRemove::ActionRemove(const Path& instancePath)
:	m_instancePath(instancePath)
{
}

bool ActionRemove::execute(Context* context)
{
	Path instanceObjectPath = getInstanceObjectPath(m_instancePath);
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);

	Ref< LocalInstanceMeta > instanceMeta = readPhysicalObject< LocalInstanceMeta >(instanceMetaPath);
	if (!instanceMeta)
		return false;

	const std::vector< std::wstring >& blobs = instanceMeta->getBlobs();
	for (std::vector< std::wstring >::const_iterator i = blobs.begin(); i != blobs.end(); ++i)
	{
		Path instanceDataPath = getInstanceDataPath(m_instancePath, *i);
		if (FileSystem::getInstance().move(
			instanceDataPath.getPathName() + L"~",
			instanceDataPath,
			true
		))
			m_renamedFiles.push_back(instanceDataPath.getPathName());
		else
			return false;
	}

	if (FileSystem::getInstance().move(
		instanceObjectPath.getPathName() + L"~",
		instanceObjectPath,
		true
	))
		m_renamedFiles.push_back(instanceObjectPath.getPathName());
	else
		return false;

	if (FileSystem::getInstance().move(
		instanceMetaPath.getPathName() + L"~",
		instanceMetaPath,
		true
	))
		m_renamedFiles.push_back(instanceMetaPath.getPathName());
	else
		return false;

	return true;
}

bool ActionRemove::undo(Context* context)
{
	for (std::vector< std::wstring >::const_iterator i = m_renamedFiles.begin(); i != m_renamedFiles.end(); ++i)
	{
		if (!FileSystem::getInstance().move(
			*i,
			*i + L"~",
			true
		))
			return false;
	}

	m_renamedFiles.clear();
	return true;
}

void ActionRemove::clean(Context* context)
{
	for (std::vector< std::wstring >::const_iterator i = m_renamedFiles.begin(); i != m_renamedFiles.end(); ++i)
		FileSystem::getInstance().remove(*i + L"~");
}

	}
}
