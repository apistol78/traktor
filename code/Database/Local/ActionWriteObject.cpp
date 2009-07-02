#include "Database/Local/ActionWriteObject.h"
#include "Database/Local/Context.h"
#include "Database/Local/PhysicalAccess.h"
#include "Database/Local/LocalInstanceMeta.h"
#include "Core/Io/FileSystem.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.ActionWriteObject", ActionWriteObject, Action)

ActionWriteObject::ActionWriteObject(const Path& instancePath, const Serializable* object)
:	m_instancePath(instancePath)
,	m_object(object)
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
			std::wstring(instanceObjectPath) + L"~",
			instanceObjectPath,
			true
		);
		if (!m_oldObjectRenamed)
			return false;
	}

	Ref< LocalInstanceMeta > instanceMeta = readPhysicalObject< LocalInstanceMeta >(instanceMetaPath);
	if (!instanceMeta)
		return false;

	if (!writePhysicalObject(instanceObjectPath, m_object, context->preferBinary()))
		return false;

	// Write meta data if primary type has changed.
	std::wstring primaryTypeName = type_name(m_object);
	if (instanceMeta->getPrimaryType() != primaryTypeName)
	{
		m_oldMetaRenamed = FileSystem::getInstance().move(
			std::wstring(instanceMetaPath) + L"~",
			instanceMetaPath,
			true
		);
		if (!m_oldMetaRenamed)
			return false;

		instanceMeta->setPrimaryType(type_name(m_object));

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
			std::wstring(instanceObjectPath) + L"~",
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
			std::wstring(instanceMetaPath) + L"~",
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
		FileSystem::getInstance().remove(std::wstring(instanceObjectPath) + L"~");
	}
	if (m_oldMetaRenamed)
	{
		Path instanceMetaPath = getInstanceMetaPath(m_instancePath);
		FileSystem::getInstance().remove(std::wstring(instanceMetaPath) + L"~");
	}
}

	}
}
