#include "Database/Local/ActionSetGuid.h"
#include "Database/Local/Context.h"
#include "Database/Local/PhysicalAccess.h"
#include "Database/Local/LocalInstanceMeta.h"
#include "Core/Io/FileSystem.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.ActionSetGuid", ActionSetGuid, Action)

ActionSetGuid::ActionSetGuid(const Path& instancePath, const Guid& newGuid, bool create)
:	m_instancePath(instancePath)
,	m_newGuid(newGuid)
,	m_create(create)
,	m_renamedMeta(false)
{
}

bool ActionSetGuid::execute(Context* context)
{
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);

	Ref< LocalInstanceMeta > instanceMeta;
	
	if (!m_create)
	{
		instanceMeta = readPhysicalObject< LocalInstanceMeta >(instanceMetaPath);
		if (!instanceMeta)
			return false;

		m_renamedMeta = FileSystem::getInstance().move(
			instanceMetaPath.getPathName() + L"~",
			instanceMetaPath,
			true
		);
		if (!m_renamedMeta)
			return false;
	}
	else
		instanceMeta = gc_new< LocalInstanceMeta >();

	instanceMeta->setGuid(m_newGuid);

	if (!writePhysicalObject(instanceMetaPath, instanceMeta, context->preferBinary()))
		return false;

	return true;
}

bool ActionSetGuid::undo(Context* context)
{
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);
	if (m_renamedMeta)
	{
		if (!FileSystem::getInstance().move(
			instanceMetaPath,
			instanceMetaPath.getPathName() + L"~",
			true
		))
			return false;

		m_renamedMeta = false;
	}
	else if (m_create)
	{
		return FileSystem::getInstance().remove(
			instanceMetaPath
		);
	}
	return true;
}

void ActionSetGuid::clean(Context* context)
{
	if (m_renamedMeta)
	{
		Path instanceMetaPath = getInstanceMetaPath(m_instancePath);
		FileSystem::getInstance().remove(
			instanceMetaPath.getPathName() + L"~"
		);
	}
}

	}
}
