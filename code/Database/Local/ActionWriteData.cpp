#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/FileSystem.h"
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
		true
	);
}

bool ActionWriteData::execute(Context* context)
{
	Ref< IFileStore > fileStore = context->getFileStore();
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);
	Path instanceDataPath = getInstanceDataPath(m_instancePath, m_dataName);

	Ref< LocalInstanceMeta > instanceMeta = readPhysicalObject< LocalInstanceMeta >(instanceMetaPath);
	if (!instanceMeta)
		return false;

	m_existingBlob = instanceMeta->haveBlob(m_dataName);

	if (m_existingBlob)
	{
		if (!fileStore->edit(instanceDataPath))
			return false;
	}

	int dataBufferSize = int(m_dataBuffer.size());
	if (dataBufferSize > 0)
	{
		Ref< IStream > writeStream = FileSystem::getInstance().open(instanceDataPath, File::FmWrite);
		if (!writeStream)
			return false;

		if (writeStream->write(&m_dataBuffer[0], dataBufferSize) != dataBufferSize)
			return false;

		writeStream->close();
		writeStream = 0;
	}

	if (!m_existingBlob)
	{
		if (!fileStore->edit(instanceMetaPath))
			return false;

		instanceMeta->addBlob(m_dataName);
		if (!writePhysicalObject(instanceMetaPath, instanceMeta, context->preferBinary()))
			return false;

		if (!fileStore->add(instanceDataPath))
			return false;
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
