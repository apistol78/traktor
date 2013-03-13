#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
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

	if (m_existingBlob)
	{
		if (!fileStore->edit(instanceDataPath))
		{
			log::error << L"Unable to open \"" << instanceDataPath.getPathName() << L"\" for edit." << Endl;
			return false;
		}
	}

	m_dataStream->close();
	m_dataStream = 0;

	Ref< IStream > writeStream = FileSystem::getInstance().open(instanceDataPath, File::FmWrite);
	if (!writeStream)
	{
		log::error << L"Unable to open file \"" << instanceDataPath.getPathName() << L"\"." << Endl;
		return false;
	}

	int dataBufferSize = int(m_dataBuffer.size());
	if (dataBufferSize > 0)
	{
		if (writeStream->write(&m_dataBuffer[0], dataBufferSize) != dataBufferSize)
		{
			log::error << L"Unable to write " << dataBufferSize << L" byte(s) to file \"" << instanceDataPath.getPathName() << L"\"." << Endl;
			return false;
		}
	}

	writeStream->close();
	writeStream = 0;

	if (!m_existingBlob)
	{
		if (!fileStore->edit(instanceMetaPath))
		{
			log::error << L"Unable to open \"" << instanceMetaPath.getPathName() << L"\" for edit." << Endl;
			return false;
		}

		instanceMeta->addBlob(m_dataName);
		if (!writePhysicalObject(instanceMetaPath, instanceMeta, context->preferBinary()))
		{
			log::error << L"Unable to write instance meta \"" << instanceMetaPath.getPathName() << L"\"." << Endl;
			return false;
		}

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

	}
}
