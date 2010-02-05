#include "Database/Local/ActionSetGuid.h"
#include "Database/Local/Context.h"
#include "Database/Local/IFileStore.h"
#include "Database/Local/LocalInstanceMeta.h"
#include "Database/Local/PhysicalAccess.h"
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
,	m_editMeta(false)
{
}

bool ActionSetGuid::execute(Context* context)
{
	Ref< IFileStore > fileStore = context->getFileStore();
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);
	Ref< LocalInstanceMeta > instanceMeta;
	
	if (!m_create)
	{
		instanceMeta = readPhysicalObject< LocalInstanceMeta >(instanceMetaPath);
		if (!instanceMeta)
			return false;

		m_editMeta = fileStore->edit(instanceMetaPath);
		if (!m_editMeta)
			return false;
	}
	else
		instanceMeta = new LocalInstanceMeta();

	instanceMeta->setGuid(m_newGuid);

	if (!writePhysicalObject(instanceMetaPath, instanceMeta, context->preferBinary()))
		return false;

	if (m_create)
		fileStore->add(instanceMetaPath);

	return true;
}

bool ActionSetGuid::undo(Context* context)
{
	Ref< IFileStore > fileStore = context->getFileStore();
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);

	if (m_editMeta)
	{
		fileStore->rollback(instanceMetaPath);
		m_editMeta = false;
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
	Ref< IFileStore > fileStore = context->getFileStore();
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);

	if (m_editMeta)
		fileStore->clean(instanceMetaPath);
}

	}
}
