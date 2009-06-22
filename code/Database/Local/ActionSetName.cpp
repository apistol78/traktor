#include "Database/Local/ActionSetName.h"
#include "Database/Local/Context.h"
#include "Database/Local/PhysicalAccess.h"
#include "Database/Local/LocalInstanceMeta.h"
#include "Core/Io/FileSystem.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.ActionSetName", ActionSetName, Action)

ActionSetName::ActionSetName(const Path& instancePath, const std::wstring& newName)
:	m_instancePath(instancePath)
,	m_newName(newName)
,	m_renamedMeta(false)
,	m_renamedObject(false)
{
}

bool ActionSetName::execute(Context* context)
{
	Path oldInstanceMetaPath = getInstanceMetaPath(m_instancePath);
	Path oldInstanceObjectPath = getInstanceObjectPath(m_instancePath);

	Path newInstancePath = m_instancePath.getPathOnly() + L"/" + m_newName;
	Path newInstanceMetaPath = getInstanceMetaPath(newInstancePath);
	Path newInstanceObjectPath = getInstanceObjectPath(newInstancePath);

	if (!FileSystem::getInstance().move(newInstanceMetaPath, oldInstanceMetaPath))
		return false;
	if (!FileSystem::getInstance().move(newInstanceObjectPath, oldInstanceObjectPath))
		return false;

	return true;
}

bool ActionSetName::undo(Context* context)
{
	Path oldInstanceMetaPath = getInstanceMetaPath(m_instancePath);
	Path oldInstanceObjectPath = getInstanceObjectPath(m_instancePath);

	Path newInstancePath = m_instancePath.getPathOnly() + L"/" + m_newName;
	Path newInstanceMetaPath = getInstanceMetaPath(newInstancePath);
	Path newInstanceObjectPath = getInstanceObjectPath(newInstancePath);

	if (m_renamedMeta)
	{
		if (!FileSystem::getInstance().move(oldInstanceMetaPath, newInstanceMetaPath, true))
			return false;
	}
	if (m_renamedObject)
	{
		if (!FileSystem::getInstance().move(oldInstanceObjectPath, newInstanceObjectPath, true))
			return false;
	}

	return true;
}

void ActionSetName::clean(Context* context)
{
}

	}
}
