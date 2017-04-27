/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Database/Local/ActionRemoveAllData.h"
#include "Database/Local/Context.h"
#include "Database/Local/IFileStore.h"
#include "Database/Local/LocalInstanceMeta.h"
#include "Database/Local/PhysicalAccess.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.ActionRemoveAllData", ActionRemoveAllData, Action)

ActionRemoveAllData::ActionRemoveAllData(const Path& instancePath)
:	m_instancePath(instancePath)
{
}

bool ActionRemoveAllData::execute(Context* context)
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

	return true;
}

bool ActionRemoveAllData::undo(Context* context)
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

void ActionRemoveAllData::clean(Context* context)
{
	Ref< IFileStore > fileStore = context->getFileStore();
	for (std::vector< std::wstring >::const_iterator i = m_renamedFiles.begin(); i != m_renamedFiles.end(); ++i)
		fileStore->clean(*i);
}

bool ActionRemoveAllData::redundant(const Action* action) const
{
	if (const ActionRemoveAllData* actionRemoveAllData = dynamic_type_cast< const ActionRemoveAllData* >(action))
		return m_instancePath == actionRemoveAllData->m_instancePath;
	else
		return false;
}

	}
}
