#include <algorithm>
#include "Database/Local/LocalInstance.h"
#include "Database/Local/LocalInstanceMeta.h"
#include "Database/Local/LocalContext.h"
#include "Database/Local/DataAccess.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.LocalInstance", LocalInstance, IProviderInstance)

LocalInstance::LocalInstance(LocalContext* context)
:	m_context(context)
,	m_locked(false)
{
}

bool LocalInstance::internalCreateExisting(const Path& instancePath)
{
	m_instancePath = instancePath;
	return true;
}

bool LocalInstance::internalCreateNew(const Path& instancePath, const Guid& instanceGuid)
{
	if (!m_context->getDataAccess()->createInstance(instancePath, instanceGuid))
		return false;

	m_instancePath = instancePath;
	return true;
}

std::wstring LocalInstance::getName() const
{
	return m_instancePath.getFileNameNoExtension();
}

Guid LocalInstance::getGuid() const
{
	Ref< LocalInstanceMeta > instanceMeta = m_context->getDataAccess()->getInstanceMeta(m_instancePath);
	return instanceMeta ? instanceMeta->getGuid() : Guid();
}

std::wstring LocalInstance::getPrimaryTypeName() const
{
	Ref< LocalInstanceMeta > instanceMeta = m_context->getDataAccess()->getInstanceMeta(m_instancePath);
	return instanceMeta ? instanceMeta->getPrimaryType() : L"";
}

bool LocalInstance::rename(const std::wstring& name)
{
	if (!m_locked)
		return false;

	if (!m_context->getDataAccess()->renameInstance(m_instancePath, name))
		return false;

	m_instancePath = m_instancePath.getPathOnly() + L"/" + name;
	return true;
}

bool LocalInstance::remove()
{
	if (!m_locked)
		return false;

	return m_context->getDataAccess()->removeInstance(m_instancePath);
}

bool LocalInstance::lock()
{
	if (m_locked)
		return false;

	m_locked = true;
	return true;
}

bool LocalInstance::unlock()
{
	if (!m_locked)
		return false;

	m_locked = false;
	return true;
}

Serializable* LocalInstance::readObject()
{
	return m_context->getDataAccess()->readObject(m_instancePath);
}

bool LocalInstance::writeObject(Serializable* object)
{
	if (!m_locked)
		return false;

	return m_context->getDataAccess()->writeObject(m_instancePath, object);
}

uint32_t LocalInstance::getDataNames(std::vector< std::wstring >& outDataNames) const
{
	return m_context->getDataAccess()->enumerateDataNames(m_instancePath, outDataNames);
}

Stream* LocalInstance::readData(const std::wstring& dataName)
{
	return m_context->getDataAccess()->readData(m_instancePath, dataName);
}

Stream* LocalInstance::writeData(const std::wstring& dataName)
{
	if (!m_locked)
		return 0;

	return m_context->getDataAccess()->writeData(m_instancePath, dataName);
}

	}
}
