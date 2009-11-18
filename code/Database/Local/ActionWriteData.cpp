#include "Database/Local/ActionWriteData.h"
#include "Database/Local/Context.h"
#include "Database/Local/PhysicalAccess.h"
#include "Database/Local/LocalInstanceMeta.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/DynamicMemoryStream.h"

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
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);
	Path instanceDataPath = getInstanceDataPath(m_instancePath, m_dataName);

	Ref< LocalInstanceMeta > instanceMeta = readPhysicalObject< LocalInstanceMeta >(instanceMetaPath);
	if (!instanceMeta)
		return false;

	m_existingBlob = instanceMeta->haveBlob(m_dataName);

	// Rename existing blob if we need to undo this operation.
	if (m_existingBlob)
	{
		if (!FileSystem::getInstance().move(
			instanceDataPath.getPathName() + L"~",
			instanceDataPath,
			true
		))
			return false;
	}

	// Write blob content.
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

	// New blob; add to meta data.
	if (!m_existingBlob)
	{
		instanceMeta->addBlob(m_dataName);
		if (!writePhysicalObject(instanceMetaPath, instanceMeta, context->preferBinary()))
			return false;
	}

	return true;
}

bool ActionWriteData::undo(Context* context)
{
	if (m_existingBlob)
	{
		// Restore old blob.
		Path instanceDataPath = getInstanceDataPath(m_instancePath, m_dataName);
		return FileSystem::getInstance().move(
			instanceDataPath,
			instanceDataPath.getPathName() + L"~",
			true
		);

		m_existingBlob = false;
	}
	else
	{
		// Remove blob from meta data.
		Path instanceMetaPath = getInstanceMetaPath(m_instancePath);
		Ref< LocalInstanceMeta > instanceMeta = readPhysicalObject< LocalInstanceMeta >(instanceMetaPath);
		if (!instanceMeta)
			return false;
		instanceMeta->removeBlob(m_dataName);
		return writePhysicalObject(instanceMetaPath, instanceMeta, context->preferBinary());
	}
}

void ActionWriteData::clean(Context* context)
{
	// Remove old, temporary, blob.
	if (m_existingBlob)
	{
		Path instanceDataPath = getInstanceDataPath(m_instancePath, m_dataName);
		FileSystem::getInstance().remove(instanceDataPath.getPathName() + L"~");
	}
}

	}
}
