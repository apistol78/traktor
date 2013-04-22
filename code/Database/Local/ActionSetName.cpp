#include "Core/Io/FileSystem.h"
#include "Database/Local/ActionSetName.h"
#include "Database/Local/Context.h"
#include "Database/Local/IFileStore.h"
#include "Database/Local/LocalInstanceMeta.h"
#include "Database/Local/PhysicalAccess.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.ActionSetName", ActionSetName, Action)

ActionSetName::ActionSetName(const Path& instancePath, const std::wstring& newName)
:	m_instancePath(instancePath)
,	m_instancePathNew(instancePath.getPathOnly() + L"/" + newName)
,	m_removedMeta(false)
,	m_removedObject(false)
{
}

bool ActionSetName::execute(Context* context)
{
	Ref< IFileStore > fileStore = context->getFileStore();

	Path oldInstanceMetaPath = getInstanceMetaPath(m_instancePath);
	Path oldInstanceObjectPath = getInstanceObjectPath(m_instancePath);

	Path newInstanceMetaPath = getInstanceMetaPath(m_instancePathNew);
	Path newInstanceObjectPath = getInstanceObjectPath(m_instancePathNew);

	if (!FileSystem::getInstance().copy(newInstanceMetaPath, oldInstanceMetaPath))
		return false;

	if (!FileSystem::getInstance().copy(newInstanceObjectPath, oldInstanceObjectPath))
		return false;

	if (!fileStore->add(newInstanceMetaPath) || !fileStore->add(newInstanceObjectPath))
		return false;

	m_removedMeta = fileStore->remove(oldInstanceMetaPath);
	m_removedObject = fileStore->remove(oldInstanceObjectPath);

	return true;
}

bool ActionSetName::undo(Context* context)
{
	Ref< IFileStore > fileStore = context->getFileStore();

	Path oldInstanceMetaPath = getInstanceMetaPath(m_instancePath);
	Path oldInstanceObjectPath = getInstanceObjectPath(m_instancePath);

	Path newInstanceMetaPath = getInstanceMetaPath(m_instancePathNew);
	Path newInstanceObjectPath = getInstanceObjectPath(m_instancePathNew);

	if (m_removedMeta)
		fileStore->rollback(oldInstanceMetaPath);

	if (m_removedObject)
		fileStore->rollback(oldInstanceObjectPath);

	m_removedMeta = false;
	m_removedObject = false;

	return true;
}

void ActionSetName::clean(Context* context)
{
	Ref< IFileStore > fileStore = context->getFileStore();

	Path oldInstanceMetaPath = getInstanceMetaPath(m_instancePath);
	Path oldInstanceObjectPath = getInstanceObjectPath(m_instancePath);

	if (m_removedMeta)
		fileStore->clean(oldInstanceMetaPath);

	if (m_removedObject)
		fileStore->clean(oldInstanceObjectPath);
}

bool ActionSetName::redundant(const Action* action) const
{
	if (const ActionSetName* actionSetName = dynamic_type_cast< const ActionSetName* >(action))
		return m_instancePath == actionSetName->m_instancePath;
	else
		return false;
}

	}
}
