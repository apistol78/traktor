#include "Database/Local/ActionWriteObject.h"
#include "Database/Local/Context.h"
#include "Database/Local/PhysicalAccess.h"
#include "Database/Local/LocalInstanceMeta.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/FileSystem.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.ActionWriteObject", ActionWriteObject, Action)

ActionWriteObject::ActionWriteObject(const Path& instancePath, const std::wstring& primaryTypeName, DynamicMemoryStream* objectStream)
:	m_instancePath(instancePath)
,	m_primaryTypeName(primaryTypeName)
,	m_objectStream(objectStream)
,	m_oldObjectRenamed(false)
,	m_oldMetaRenamed(false)
{
}

bool ActionWriteObject::execute(Context* context)
{
	Path instanceObjectPath = getInstanceObjectPath(m_instancePath);
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);

	if (FileSystem::getInstance().exist(instanceObjectPath))
	{
		m_oldObjectRenamed = FileSystem::getInstance().move(
			instanceObjectPath.getPathName() + L"~",
			instanceObjectPath,
			true
		);
		if (!m_oldObjectRenamed)
			return false;
	}

	Ref< LocalInstanceMeta > instanceMeta = readPhysicalObject< LocalInstanceMeta >(instanceMetaPath);
	if (!instanceMeta)
		return false;

	Ref< Stream > objectStream = FileSystem::getInstance().open(instanceObjectPath, File::FmWrite);
	if (!objectStream)
		return false;

	const std::vector< uint8_t >& objectBuffer = m_objectStream->getBuffer();
	if (objectStream->write(&objectBuffer[0], objectBuffer.size()) != objectBuffer.size())
	{
		objectStream->close();
		return false;
	}

	objectStream->close();

	// Update meta data if primary type has changed.
	if (instanceMeta->getPrimaryType() != m_primaryTypeName)
	{
		m_oldMetaRenamed = FileSystem::getInstance().move(
			instanceMetaPath.getPathName() + L"~",
			instanceMetaPath,
			true
		);
		if (!m_oldMetaRenamed)
			return false;

		instanceMeta->setPrimaryType(m_primaryTypeName);

		if (!writePhysicalObject(instanceMetaPath, instanceMeta, context->preferBinary()))
			return false;
	}

	return true;
}

bool ActionWriteObject::undo(Context* context)
{
	if (m_oldObjectRenamed)
	{
		Path instanceObjectPath = getInstanceObjectPath(m_instancePath);
		if (!FileSystem::getInstance().move(
			instanceObjectPath,
			instanceObjectPath.getPathName() + L"~",
			true
		))
			return false;

		m_oldObjectRenamed = false;
	}
	if (m_oldMetaRenamed)
	{
		Path instanceMetaPath = getInstanceMetaPath(m_instancePath);
		if (!FileSystem::getInstance().move(
			instanceMetaPath,
			instanceMetaPath.getPathName() + L"~",
			true
		))
			return false;

		m_oldMetaRenamed = false;
	}

	return true;
}

void ActionWriteObject::clean(Context* context)
{
	if (m_oldObjectRenamed)
	{
		Path instanceObjectPath = getInstanceObjectPath(m_instancePath);
		FileSystem::getInstance().remove(instanceObjectPath.getPathName() + L"~");
	}
	if (m_oldMetaRenamed)
	{
		Path instanceMetaPath = getInstanceMetaPath(m_instancePath);
		FileSystem::getInstance().remove(instanceMetaPath.getPathName() + L"~");
	}
}

	}
}
