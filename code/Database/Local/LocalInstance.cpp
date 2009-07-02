#include <algorithm>
#include "Database/Local/LocalInstance.h"
#include "Database/Local/LocalInstanceMeta.h"
#include "Database/Local/Context.h"
#include "Database/Local/Transaction.h"
#include "Database/Local/ActionSetGuid.h"
#include "Database/Local/ActionSetName.h"
#include "Database/Local/ActionRemove.h"
#include "Database/Local/ActionWriteObject.h"
#include "Database/Local/ActionWriteData.h"
#include "Database/Local/PhysicalAccess.h"
#include "Core/Io/FileSystem.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.LocalInstance", LocalInstance, IProviderInstance)

LocalInstance::LocalInstance(Context* context)
:	m_context(context)
{
}

bool LocalInstance::internalCreate(const Path& instancePath)
{
	m_instancePath = instancePath;
	return true;
}

bool LocalInstance::internalCreateNew(const Path& instancePath, const Guid& instanceGuid)
{
	if (!internalCreate(instancePath))
		return false;

	if (!openTransaction())
		return false;

	m_transaction->add(gc_new< ActionSetGuid >(
		cref(m_instancePath),
		cref(instanceGuid),
		true
	));

	return true;
}

std::wstring LocalInstance::getPrimaryTypeName() const
{
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);
	Ref< LocalInstanceMeta > instanceMeta = readPhysicalObject< LocalInstanceMeta >(instanceMetaPath);
	return instanceMeta ? instanceMeta->getPrimaryType() : L"";
}

bool LocalInstance::openTransaction()
{
	if (m_transaction)
		return false;

	m_transaction = gc_new< Transaction >();
	m_transactionName.clear();

	return true;
}

bool LocalInstance::commitTransaction()
{
	if (!m_transaction)
		return false;
	if (!m_transaction->commit(m_context))
		return false;
	if (!m_transactionName.empty())
		m_instancePath = m_instancePath = m_instancePath.getPathOnly() + L"/" + m_transactionName;
	return true;
}

bool LocalInstance::closeTransaction()
{
	if (!m_transaction)
		return false;
	m_transaction = 0;
	return true;
}

std::wstring LocalInstance::getName() const
{
	return m_instancePath.getFileNameNoExtension();
}

bool LocalInstance::setName(const std::wstring& name)
{
	if (!m_transaction || name.empty())
		return false;

	Ref< ActionSetName > action = gc_new< ActionSetName >(cref(m_instancePath), cref(name));
	m_transaction->add(action);
	m_transactionName = name;

	return true;
}

Guid LocalInstance::getGuid() const
{
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);
	Ref< LocalInstanceMeta > instanceMeta = readPhysicalObject< LocalInstanceMeta >(instanceMetaPath);
	return instanceMeta ? instanceMeta->getGuid() : Guid();
}

bool LocalInstance::setGuid(const Guid& guid)
{
	if (!m_transaction)
		return false;

	m_transaction->add(gc_new< ActionSetGuid >(
		cref(m_instancePath), 
		cref(guid),
		false
	));

	return true;
}

bool LocalInstance::remove()
{
	if (!m_transaction)
		return false;

	m_transaction->add(gc_new< ActionRemove >(
		cref(m_instancePath)
	));

	return true;
}

Serializable* LocalInstance::getObject()
{
	Path instanceObjectPath = getInstanceObjectPath(m_instancePath);
	return readPhysicalObject(instanceObjectPath);
}

bool LocalInstance::setObject(const Serializable* object)
{
	if (!m_transaction)
		return false;

	m_transaction->add(gc_new< ActionWriteObject >(
		cref(m_instancePath),
		object
	));

	return true;
}

uint32_t LocalInstance::getDataNames(std::vector< std::wstring >& outDataNames) const
{
	Path instanceMetaPath = getInstanceMetaPath(m_instancePath);

	Ref< LocalInstanceMeta > instanceMeta = readPhysicalObject< LocalInstanceMeta >(instanceMetaPath);
	if (!instanceMeta)
		return 0;

	outDataNames = instanceMeta->getBlobs();
	return uint32_t(outDataNames.size());
}

Stream* LocalInstance::readData(const std::wstring& dataName)
{
	Path instanceDataPath = getInstanceDataPath(m_instancePath, dataName);
	return FileSystem::getInstance().open(instanceDataPath, File::FmRead);
}

Stream* LocalInstance::writeData(const std::wstring& dataName)
{
	if (!m_transaction)
		return 0;

	Ref< ActionWriteData > action = gc_new< ActionWriteData >(cref(m_instancePath), dataName);
	m_transaction->add(action);

	return action->getStream();
}

	}
}
