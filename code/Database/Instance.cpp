#include "Core/Io/BufferedStream.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Thread/Acquire.h"
#include "Database/Group.h"
#include "Database/IInstanceEventListener.h"
#include "Database/Instance.h"
#include "Database/Provider/IProviderInstance.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

namespace traktor
{
	namespace db
	{
		namespace
		{

enum CacheFlags
{
	IchName = 1,
	IchGuid = 2,
	IchPrimaryType = 4
};

enum TransactionFlags
{
	TfCreated = 1,
	TfRemoved = 2,
	TfNameChanged = 4,
	TfGuidChanged = 8,
	TfObjectChanged = 16,
	TfDataChanged = 32
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.Instance", Instance, Object)

Group* Instance::getParent() const
{
	return m_parent;
}

std::wstring Instance::getName() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (m_providerInstance);

	if (!(m_cachedFlags & IchName))
	{
		m_name = m_providerInstance->getName();
		m_cachedFlags |= IchName;
	}

	return m_name;
}

std::wstring Instance::getPath() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (m_parent);
	return m_parent->getPath() + L"/" + getName();
}

Guid Instance::getGuid() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (m_providerInstance);

	if (!(m_cachedFlags & IchGuid))
	{
		m_guid = m_providerInstance->getGuid();
		m_cachedFlags |= IchGuid;
	}

	return m_guid;
}

bool Instance::getLastModifyDate(DateTime& outModifyDate) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (m_providerInstance);
	return m_providerInstance->getLastModifyDate(outModifyDate);
}

uint32_t Instance::getFlags() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (m_providerInstance);
	return m_providerInstance->getFlags();
}

std::wstring Instance::getPrimaryTypeName() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (m_providerInstance);

	if (!(m_cachedFlags & IchPrimaryType))
	{
		m_type = m_providerInstance->getPrimaryTypeName();
		m_cachedFlags |= IchPrimaryType;
	}

	return m_type;
}

const TypeInfo* Instance::getPrimaryType() const
{
	return TypeInfo::find(getPrimaryTypeName());
}

Ref< ISerializable > Instance::getObject() const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (m_providerInstance);

	Ref< ISerializable > object;
	const TypeInfo* serializerType = 0;

	Ref< IStream > stream = m_providerInstance->readObject(serializerType);
	if (!stream || !serializerType)
		return 0;

	BufferedStream bs(stream);
	Ref< Serializer > serializer;
	if (serializerType == &type_of< BinarySerializer >())
		serializer = new BinarySerializer(&bs);
	else if (serializerType == &type_of< xml::XmlDeserializer >())
		serializer = new xml::XmlDeserializer(&bs);
	else
	{
		stream->close();
		return 0;
	}

	object = serializer->readObject();

	stream->close();
	return object;
}

uint32_t Instance::getDataNames(std::vector< std::wstring >& dataNames) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (m_providerInstance);
	return m_providerInstance->getDataNames(dataNames);
}

bool Instance::getDataLastWriteTime(const std::wstring& dataName, DateTime& outLastWriteTime) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (m_providerInstance);
	return m_providerInstance->getDataLastWriteTime(dataName, outLastWriteTime);
}

Ref< IStream > Instance::readData(const std::wstring& dataName) const
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (m_providerInstance);

	Ref< IStream > stream = m_providerInstance->readData(dataName);
	return stream ? new BufferedStream(stream) : 0;
}

bool Instance::checkout()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (m_providerInstance);

	if (!m_providerInstance->openTransaction())
		return false;

	m_transactionGuid = getGuid();
	m_transactionName = getName();
	m_transactionFlags = 0;

	return true;
}

bool Instance::commit(uint32_t flags)
{
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		T_ASSERT (m_providerInstance);

		if ((flags & CfKeepCheckedOut) != 0 && (m_transactionFlags & TfRemoved) != 0)
		{
			log::error << L"Instance commit failed; cannot keep checked out as instance was removed" << Endl;
			return false;
		}

		if (!m_providerInstance->commitTransaction())
		{
			log::error << L"Instance commit failed; commitTransaction failed" << Endl;
			return false;
		}

		if ((flags & CfKeepCheckedOut) == 0)
		{
			if (!m_providerInstance->closeTransaction())
			{
				log::error << L"Instance commit failed; closeTransaction failed" << Endl;
				return false;
			}
		}

		if ((m_transactionFlags & TfGuidChanged) != 0)
		{
			m_guid = m_providerInstance->getGuid();
			m_cachedFlags |= IchGuid;
		}

		if ((m_transactionFlags & TfNameChanged) != 0)
		{
			m_name = m_providerInstance->getName();
			m_cachedFlags |= IchName;
		}
	}

	if (m_eventListener)
	{
		if ((m_transactionFlags & TfCreated) != 0)
			m_eventListener->instanceEventCreated(this);

		if ((m_transactionFlags & TfGuidChanged) != 0)
			m_eventListener->instanceEventGuidChanged(this, m_transactionGuid);

		if ((m_transactionFlags & TfNameChanged) != 0)
			m_eventListener->instanceEventRenamed(this, m_transactionName);

		if ((m_transactionFlags & TfRemoved) != 0)
			m_eventListener->instanceEventRemoved(this);

		m_eventListener->instanceEventCommitted(this);
	}

	if ((m_transactionFlags & TfRemoved) != 0)
		internalDestroy();

	return true;
}

bool Instance::revert()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (m_providerInstance);

	if (!m_providerInstance->closeTransaction())
		return false;

	m_transactionFlags = 0;
	return true;
}

bool Instance::remove()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (m_providerInstance);

	if (!m_providerInstance->remove())
		return false;

	m_transactionFlags |= TfRemoved;
	return true;
}

bool Instance::setName(const std::wstring& name)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (m_providerInstance);

	if (!m_providerInstance->setName(name))
		return false;

	m_transactionFlags |= TfNameChanged;
	return true;
}

bool Instance::setGuid(const Guid& guid)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (m_providerInstance);

	if (!m_providerInstance->setGuid(guid))
		return false;

	m_transactionFlags |= TfGuidChanged;
	return true;
}

bool Instance::setObject(const ISerializable* object)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (m_providerInstance);

	if (!object)
		return false;

	const TypeInfo* serializerType = 0;

	Ref< IStream > stream = m_providerInstance->writeObject(type_name(object), serializerType);
	if (!stream || !serializerType)
		return false;

	Ref< Serializer > serializer;
	if (serializerType == &type_of< BinarySerializer >())
		serializer = new BinarySerializer(stream);
	else if (serializerType == &type_of< xml::XmlSerializer >())
		serializer = new xml::XmlSerializer(stream);
	else
	{
		stream->close();
		return false;
	}

	bool result = serializer->writeObject(object);

	stream->flush();
	stream->close();

	if (result)
		m_transactionFlags |= TfObjectChanged;

	return result;
}

bool Instance::removeAllData()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (m_providerInstance);
	
	if (!m_providerInstance->removeAllData())
		return false;

	m_transactionFlags |= TfDataChanged;
	return true;
}

Ref< IStream > Instance::writeData(const std::wstring& dataName)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	T_ASSERT (m_providerInstance);

	Ref< IStream > stream = m_providerInstance->writeData(dataName);
	if (stream)
		m_transactionFlags |= TfDataChanged;

	return stream;
}

Instance::Instance(IInstanceEventListener* eventListener)
:	m_eventListener(eventListener)
,	m_parent(0)
,	m_cachedFlags(0)
,	m_transactionFlags(0)
{
}

bool Instance::internalCreateExisting(IProviderInstance* providerInstance, Group* parent)
{
	m_providerInstance = providerInstance;
	m_parent = parent;
	m_cachedFlags = 0;
	m_transactionFlags = 0;
	return true;
}

bool Instance::internalCreateNew(IProviderInstance* providerInstance, Group* parent)
{
	m_providerInstance = providerInstance;
	m_parent = parent;
	m_cachedFlags = 0;
	m_transactionFlags = TfCreated;
	return true;
}

void Instance::internalDestroy()
{
	m_providerInstance = 0;
	m_parent = 0;
	m_cachedFlags = 0;
	m_transactionFlags = 0;
}

void Instance::internalFlush()
{
	m_cachedFlags = 0;
}

	}
}
