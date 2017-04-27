/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Local/ActionRemove.h"
#include "Database/Local/Context.h"
#include "Database/Local/IFileStore.h"
#include "Database/Local/LocalInstanceMeta.h"
#include "Database/Local/PhysicalAccess.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"

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
	Ref< IFileStore > fileStore = context->getFileStore();
	Path instanceObjectPath = getInstanceObjectPath(m_instancePath);
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);

	Ref< LocalInstanceMeta > instanceMeta = readPhysicalObject< LocalInstanceMeta >(instanceMetaPath);
	if (!instanceMeta)
	{
		log::error << L"Action remove failed; unable to read meta object" << Endl;
		return false;
	}

	const std::vector< std::wstring >& blobs = instanceMeta->getBlobs();
	for (std::vector< std::wstring >::const_iterator i = blobs.begin(); i != blobs.end(); ++i)
	{
		Path instanceDataPath = getInstanceDataPath(m_instancePath, *i);
		if (fileStore->remove(instanceDataPath))
			m_renamedFiles.push_back(instanceDataPath.getPathName());
		else
		{
			log::error << L"Action remove failed; unable to remove \"" << instanceDataPath.getPathName() << L"\"" << Endl;
			return false;
		}
	}

	if (fileStore->remove(instanceObjectPath))
		m_renamedFiles.push_back(instanceObjectPath.getPathName());
	else
	{
		log::error << L"Action remove failed; unable to remove \"" << instanceObjectPath.getPathName() << L"~\"" << Endl;
		return false;
	}

	if (fileStore->remove(instanceMetaPath))
		m_renamedFiles.push_back(instanceMetaPath.getPathName());
	else
	{
		log::error << L"Action remove failed; unable to remove \"" << instanceMetaPath.getPathName() << L"~\"" << Endl;
		return false;
	}

	return true;
}

bool ActionRemove::undo(Context* context)
{
	Ref< IFileStore > fileStore = context->getFileStore();

	for (std::vector< std::wstring >::const_iterator i = m_renamedFiles.begin(); i != m_renamedFiles.end(); ++i)
	{
		if (!fileStore->rollback(*i))
			return false;
	}

	m_renamedFiles.clear();
	return true;
}

void ActionRemove::clean(Context* context)
{
	Ref< IFileStore > fileStore = context->getFileStore();
	for (std::vector< std::wstring >::const_iterator i = m_renamedFiles.begin(); i != m_renamedFiles.end(); ++i)
		fileStore->clean(*i);
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
