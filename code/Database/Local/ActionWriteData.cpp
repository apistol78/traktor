/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/ChunkMemory.h"
#include "Core/Io/ChunkMemoryStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Local/ActionWriteData.h"
#include "Database/Local/Context.h"
#include "Database/Local/IFileStore.h"
#include "Database/Local/LocalInstanceMeta.h"
#include "Database/Local/PhysicalAccess.h"

namespace traktor::db
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.ActionWriteData", ActionWriteData, Action)

ActionWriteData::ActionWriteData(const Path& instancePath, const std::wstring& dataName)
:	m_instancePath(instancePath)
,	m_dataName(dataName)
,	m_existingBlob(false)
{
	m_dataMemory = new ChunkMemory();
}

bool ActionWriteData::execute(Context& context)
{
	IFileStore* fileStore = context.getFileStore();
	const Path instanceMetaPath = getInstanceMetaPath(m_instancePath);
	const Path instanceDataPath = getInstanceDataPath(m_instancePath, m_dataName);

	Ref< LocalInstanceMeta > instanceMeta = readPhysicalObject< LocalInstanceMeta >(instanceMetaPath);
	if (!instanceMeta)
	{
		log::error << L"Unable to read instance meta data, \"" << instanceMetaPath.getPathName() << L"\"." << Endl;
		return false;
	}

	m_existingBlob = instanceMeta->haveBlob(m_dataName);

	// If blob already exist then we need to open it in the file store.
	if (m_existingBlob)
	{
		if (!fileStore->edit(instanceDataPath))
		{
			log::error << L"Unable to open \"" << instanceDataPath.getPathName() << L"\" for edit." << Endl;
			return false;
		}
	}

	// Create output, physical, stream. Retry one time if initially locked.
	Ref< IStream > writeStream = FileSystem::getInstance().open(instanceDataPath, File::FmWrite);
	if (!writeStream)
	{
		ThreadManager::getInstance().getCurrentThread()->sleep(100);
		writeStream = FileSystem::getInstance().open(instanceDataPath, File::FmWrite);
		if (!writeStream)
		{
			log::error << L"Unable to open file \"" << instanceDataPath.getPathName() << L"\"." << Endl;
			return false;
		}
	}

	for (size_t offset = 0; offset < m_dataMemory->size(); )
	{
		const auto chunk = m_dataMemory->getChunk(offset);
		if (!chunk.ptr)
			break;

		const uint8_t* ptr = (const uint8_t*)chunk.ptr;
		size_t write = chunk.size;

		while (write > 0)
		{
			const int64_t written = writeStream->write(ptr, write);
			if (written < 0 || written > (int64_t)write)
			{
				log::error << L"Unable to write " << (uint32_t)write << L" byte(s) to file \"" << instanceDataPath.getPathName() << L"\"." << Endl;
				safeClose(writeStream);
				FileSystem::getInstance().remove(instanceDataPath);
				return false;				
			}
			ptr += written;
			write -= (size_t)written;
		}
		offset += chunk.size;
	}

	safeClose(writeStream);

	if (!fileStore->edit(instanceMetaPath))
	{
		log::error << L"Unable to open \"" << instanceMetaPath.getPathName() << L"\" for edit." << Endl;
		return false;
	}

	instanceMeta->setBlob(m_dataName);

	if (!writePhysicalObject(instanceMetaPath, instanceMeta, context.preferBinary()))
	{
		log::error << L"Unable to write instance meta \"" << instanceMetaPath.getPathName() << L"\"." << Endl;
		return false;
	}
	
	// If blob is new then we need to add it to file store.
	if (!m_existingBlob)
	{
		if (!fileStore->add(instanceDataPath))
		{
			log::error << L"Unable to add file \"" << instanceDataPath.getPathName() << L"\"." << Endl;
			return false;
		}
	}

	return true;
}

bool ActionWriteData::undo(Context& context)
{
	IFileStore* fileStore = context.getFileStore();
	const Path instanceMetaPath = getInstanceMetaPath(m_instancePath);
	const Path instanceDataPath = getInstanceDataPath(m_instancePath, m_dataName);

	if (m_existingBlob)
		fileStore->rollback(instanceDataPath);
	else
		fileStore->rollback(instanceMetaPath);

	return true;
}

void ActionWriteData::clean(Context& context)
{
	IFileStore* fileStore = context.getFileStore();
	const Path instanceMetaPath = getInstanceMetaPath(m_instancePath);
	const Path instanceDataPath = getInstanceDataPath(m_instancePath, m_dataName);

	if (m_existingBlob)
		fileStore->clean(instanceDataPath);
	else
		fileStore->clean(instanceMetaPath);
}

bool ActionWriteData::redundant(const Action* action) const
{
	const ActionWriteData* actionWriteData = dynamic_type_cast< const ActionWriteData* >(action);
	if (actionWriteData)
	{
		return
			m_instancePath == actionWriteData->m_instancePath &&
			m_dataName == actionWriteData->m_dataName;
	}
	else
		return false;
}

Ref< IStream > ActionWriteData::getWriteStream() const
{
	return new ChunkMemoryStream(m_dataMemory, false, true);
}

Ref< IStream > ActionWriteData::getReadStream() const
{
	return new ChunkMemoryStream(m_dataMemory, true, false);
}

}
