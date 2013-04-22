#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Database/Local/ActionSetGuid.h"
#include "Database/Local/Context.h"
#include "Database/Local/IFileStore.h"
#include "Database/Local/LocalInstanceMeta.h"
#include "Database/Local/PhysicalAccess.h"

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
		{
			log::error << L"Unable to read instance meta data" << Endl;
			return false;
		}

		m_editMeta = fileStore->edit(instanceMetaPath);
		if (!m_editMeta)
		{
			log::error << L"Unable to edit instance meta" << Endl;
			return false;
		}
	}
	else
		instanceMeta = new LocalInstanceMeta();

	instanceMeta->setGuid(m_newGuid);

	if (!writePhysicalObject(instanceMetaPath, instanceMeta, context->preferBinary()))
	{
		log::error << L"Unable to write instance meta data" << Endl;
		return false;
	}

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

bool ActionSetGuid::redundant(const Action* action) const
{
	return false;
}

	}
}
