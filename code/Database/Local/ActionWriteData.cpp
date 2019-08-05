#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/SHA1.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Local/ActionWriteData.h"
#include "Database/Local/Context.h"
#include "Database/Local/IFileStore.h"
#include "Database/Local/LocalInstanceMeta.h"
#include "Database/Local/PhysicalAccess.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.ActionWriteData", ActionWriteData, Action)

ActionWriteData::ActionWriteData(const Path& instancePath, const std::wstring& dataName)
:	m_instancePath(instancePath)
,	m_dataName(dataName)
,	m_existingBlob(false)
{
	m_dataBuffer.reserve(1 * 1024 * 1024);
	m_dataStream = new DynamicMemoryStream(
		m_dataBuffer,
		false,
		true,
		T_FILE_LINE
	);
}

bool ActionWriteData::execute(Context* context)
{
	Ref< IFileStore > fileStore = context->getFileStore();
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);
	Path instanceDataPath = getInstanceDataPath(m_instancePath, m_dataName);

	Ref< LocalInstanceMeta > instanceMeta = readPhysicalObject< LocalInstanceMeta >(instanceMetaPath);
	if (!instanceMeta)
	{
		log::error << L"Unable to read instance meta \"" << instanceMetaPath.getPathName() << L"\"." << Endl;
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

	safeClose(m_dataStream);

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

	// Calculate SHA1 hash of data while writing data.
	SHA1 sha1;
	sha1.begin();

	int64_t dataBufferSize = int64_t(m_dataBuffer.size());
	if (dataBufferSize > 0)
	{
		sha1.feed(&m_dataBuffer[0], dataBufferSize);
		if (writeStream->write(&m_dataBuffer[0], dataBufferSize) != dataBufferSize)
		{
			log::error << L"Unable to write " << dataBufferSize << L" byte(s) to file \"" << instanceDataPath.getPathName() << L"\"." << Endl;
			return false;
		}
	}

	sha1.end();

	safeClose(writeStream);

	if (!fileStore->edit(instanceMetaPath))
	{
		log::error << L"Unable to open \"" << instanceMetaPath.getPathName() << L"\" for edit." << Endl;
		return false;
	}

	instanceMeta->setBlob(m_dataName, sha1.format());

	if (!writePhysicalObject(instanceMetaPath, instanceMeta, context->preferBinary()))
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

bool ActionWriteData::undo(Context* context)
{
	Ref< IFileStore > fileStore = context->getFileStore();
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);
	Path instanceDataPath = getInstanceDataPath(m_instancePath, m_dataName);

	if (m_existingBlob)
	{
		fileStore->rollback(instanceDataPath);
	}
	else
	{
		fileStore->rollback(instanceMetaPath);
	}

	return true;
}

void ActionWriteData::clean(Context* context)
{
	Ref< IFileStore > fileStore = context->getFileStore();
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);
	Path instanceDataPath = getInstanceDataPath(m_instancePath, m_dataName);

	if (m_existingBlob)
	{
		fileStore->clean(instanceDataPath);
	}
	else
	{
		fileStore->clean(instanceMetaPath);
	}
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

	}
}
