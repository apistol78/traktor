#include "Database/Instance.h"
#include "Database/Group.h"
#include "Database/Provider.h"
#include "Core/Serialization/Serializable.h"
#include "Core/Io/Stream.h"
#include "Core/Thread/Acquire.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.Instance", Instance, Object)

Instance::Instance(IProviderBus* providerBus)
:	m_providerBus(providerBus)
{
}

bool Instance::internalCreate(IProviderInstance* providerInstance, Group* parent, Serializable* exclusiveObject)
{
	m_providerInstance = providerInstance;
	m_parent = parent;
	
	m_name = m_providerInstance->getName();
	m_guid = m_providerInstance->getGuid();
	m_primaryTypeName = m_providerInstance->getPrimaryTypeName();
	m_exclusiveObject = exclusiveObject;

	return true;
}

void Instance::internalDestroy()
{
	m_providerBus = 0;
	m_providerInstance = 0;
	m_parent = 0;
	m_name = L"";
	m_guid = Guid();
	m_primaryTypeName = L"";
	m_exclusiveObject = 0;

	Heap::getInstance().invalidateRefs(this);
}

std::wstring Instance::getName() const
{
	T_ASSERT (m_providerInstance);
	return m_name;
}

std::wstring Instance::getPath() const
{
	T_ASSERT (m_providerInstance);
	return m_parent->getPath() + L"/" + m_name;
}

Guid Instance::getGuid() const
{
	T_ASSERT (m_providerInstance);
	return m_guid;
}

std::wstring Instance::getPrimaryTypeName() const
{
	T_ASSERT (m_providerInstance);
	return m_primaryTypeName;
}

const Type* Instance::getPrimaryType() const
{
	T_ASSERT (m_providerInstance);
	return Type::find(m_primaryTypeName);
}

bool Instance::lock()
{
	T_ASSERT (m_providerInstance);
	return m_providerInstance->lock();
}

bool Instance::unlock()
{
	T_ASSERT (m_providerInstance);
	return m_providerInstance->unlock();
}

bool Instance::rename(const std::wstring& name)
{
	T_ASSERT (m_providerInstance);
	
	if (!m_providerInstance->rename(name))
		return false;

	if (m_providerBus)
		m_providerBus->putEvent(PeRenamed, getGuid());

	m_name = name;
	return true;
}

bool Instance::remove()
{
	T_ASSERT (m_providerInstance);

	if (!m_providerInstance->remove())
		return false;

	m_parent->removeChildInstance(this);

	if (m_providerBus)
		m_providerBus->putEvent(PeRemoved, getGuid());

	internalDestroy();
	return true;
}

Serializable* Instance::checkout(uint32_t flags)
{
	T_ASSERT (m_providerInstance);

	Acquire< Mutex > __lock__(m_lock);

	if (flags & CfExclusive)
	{
		if (m_exclusiveObject)
			return 0;
		if (!m_providerInstance->lock())
			return 0;
	}

	Ref< Serializable > resultObject = m_providerInstance->readObject();
	if (!resultObject)
	{
		if (flags & CfExclusive)
			m_providerInstance->unlock();
		return 0;
	}

	if (flags & CfExclusive)
		m_exclusiveObject = resultObject;

	return resultObject;
}

bool Instance::replace(Serializable* object)
{
	T_ASSERT (m_providerInstance);

	Acquire< Mutex > __lock__(m_lock);

	if (!m_exclusiveObject)
		return false;

	m_exclusiveObject = object;
	return true;
}

bool Instance::commit(uint32_t flags)
{
	T_ASSERT (m_providerInstance);

	Acquire< Mutex > __lock__(m_lock);

	if (!m_exclusiveObject)
		return false;

	bool result = m_providerInstance->writeObject(m_exclusiveObject);
	if (!result)
		return false;

	if (!(flags & CfKeepCheckedOut))
	{
		m_exclusiveObject = 0;
		m_providerInstance->unlock();
	}

	m_primaryTypeName = m_providerInstance->getPrimaryTypeName();

	if (m_providerBus)
		m_providerBus->putEvent(PeCommited, getGuid());

	return true;
}

bool Instance::revert()
{
	T_ASSERT (m_providerInstance);

	Acquire< Mutex > __lock__(m_lock);

	if (!m_exclusiveObject)
		return false;

	m_exclusiveObject = 0;
	m_providerInstance->unlock();

	if (m_providerBus)
		m_providerBus->putEvent(PeReverted, getGuid());

	return true;
}

uint32_t Instance::getDataNames(std::vector< std::wstring >& dataNames) const
{
	T_ASSERT (m_providerInstance);
	return m_providerInstance->getDataNames(dataNames);
}

Stream* Instance::readData(const std::wstring& dataName)
{
	T_ASSERT (m_providerInstance);
	return m_providerInstance->readData(dataName);
}

Stream* Instance::writeData(const std::wstring& dataName)
{
	T_ASSERT (m_providerInstance);
	return m_providerInstance->writeData(dataName);
}

Group* Instance::getParent()
{
	return m_parent;
}

	}
}
