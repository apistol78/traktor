#include "Core/Io/BufferedStream.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Thread/Acquire.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Provider/IProviderBus.h"
#include "Database/Provider/IProviderInstance.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

namespace traktor
{
	namespace db
	{
		namespace
		{

enum
{
	IchName = 1,
	IchGuid = 2,
	IchPrimaryType = 4
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.Instance", Instance, Object)

Instance::Instance(Database* database)
:	m_database(database)
,	m_providerBus(0)
,	m_parent(0)
,	m_renamed(false)
,	m_removed(false)
#if T_INSTANCE_CACHE_NAME || T_INSTANCE_CACHE_GUID || T_INSTANCE_CACHE_PRIMARY_TYPE
,	m_cache(0)
#endif
{
}

bool Instance::internalCreate(IProviderBus* providerBus, IProviderInstance* providerInstance, Group* parent)
{
	T_FATAL_ASSERT_M(providerInstance, L"No provider instance");
	m_providerBus = providerBus;
	m_providerInstance = providerInstance;
	m_parent = parent;
	return true;
}

void Instance::internalDestroy()
{
	m_providerBus = 0;
	m_providerInstance = 0;
	m_parent = 0;
	m_renamed = false;
	m_removed = false;
#if T_INSTANCE_CACHE_NAME || T_INSTANCE_CACHE_GUID || T_INSTANCE_CACHE_PRIMARY_TYPE
	m_cache = 0;
#endif
}

void Instance::internalReset()
{
#if T_INSTANCE_CACHE_NAME || T_INSTANCE_CACHE_GUID || T_INSTANCE_CACHE_PRIMARY_TYPE
	m_cache = 0;
#endif
}

std::wstring Instance::getName() const
{
	T_ASSERT (m_providerInstance);
#if T_INSTANCE_CACHE_NAME
	if (!(m_cache & IchName))
	{
		m_name = m_providerInstance->getName();
		m_cache |= IchName;
	}
	return m_name;
#else
	return m_providerInstance->getName();
#endif
}

std::wstring Instance::getPath() const
{
	T_ASSERT (m_providerInstance);
	return m_parent->getPath() + L"/" + getName();
}

Guid Instance::getGuid() const
{
	T_ASSERT (m_providerInstance);
#if T_INSTANCE_CACHE_GUID
	if (!(m_cache & IchGuid))
	{
		m_guid = m_providerInstance->getGuid();
		m_cache |= IchGuid;
	}
	return m_guid;
#else
	return m_providerInstance->getGuid();
#endif
}

bool Instance::setGuid(const Guid& guid)
{
	T_ASSERT (m_providerInstance);

	if (m_database)
		m_database->flushInstance(getGuid());

	if (!m_providerInstance->setGuid(guid))
		return false;

#if T_INSTANCE_CACHE_GUID
	m_cache &= ~IchGuid;
#endif
	return true;
}

std::wstring Instance::getPrimaryTypeName() const
{
	T_ASSERT (m_providerInstance);
#if T_INSTANCE_CACHE_PRIMARY_TYPE
	if (!(m_cache & IchPrimaryType))
	{
		m_primaryType = m_providerInstance->getPrimaryTypeName();
		m_cache |= IchPrimaryType;
	}
	return m_primaryType;
#else
	return m_providerInstance->getPrimaryTypeName();
#endif
}

const TypeInfo* Instance::getPrimaryType() const
{
	T_ASSERT (m_providerInstance);
	return TypeInfo::find(getPrimaryTypeName());
}

bool Instance::checkout()
{
	T_ASSERT (m_providerInstance);

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!m_providerInstance->openTransaction())
		return false;

	m_renamed = false;
	m_removed = false;

	return true;
}

bool Instance::commit(uint32_t flags)
{
	T_ASSERT (m_providerInstance);

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if ((flags & CfKeepCheckedOut) != 0 && m_removed)
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

	Guid guid = getGuid();

	if (m_providerBus)
	{
		if (!m_removed && !m_renamed)
			m_providerBus->putEvent(PeCommited, guid);
		else
		{
			if (m_removed)
				m_providerBus->putEvent(PeRemoved, guid);
			if (m_renamed)
				m_providerBus->putEvent(PeRenamed, guid);
		}
	}

#if T_INSTANCE_CACHE_NAME
	if (m_renamed)
	{
		m_name = m_providerInstance->getName();
		m_cache |= IchName;
	}
#endif

	if (m_removed)
	{
		if (m_parent)
			m_parent->removeChildInstance(this);
		
		if (m_database)
			m_database->flushInstance(guid);

		internalDestroy();
	}

	return true;
}

bool Instance::revert()
{
	T_ASSERT (m_providerInstance);

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!m_providerInstance->closeTransaction())
		return false;

	if (m_providerBus)
		m_providerBus->putEvent(PeReverted, getGuid());

	return true;
}

bool Instance::setName(const std::wstring& name)
{
	T_ASSERT (m_providerInstance);

	if (!m_providerInstance->setName(name))
		return false;

	m_renamed = true;
	return true;
}

bool Instance::remove()
{
	T_ASSERT (m_providerInstance);

	if (!m_providerInstance->remove())
		return false;

	m_removed = true;
	return true;
}

Ref< ISerializable > Instance::getObject()
{
	T_ASSERT (m_providerInstance);

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	Ref< ISerializable > object;
	const TypeInfo* serializerType = 0;

	Ref< IStream > stream = m_providerInstance->readObject(serializerType);
	if (!stream)
		return 0;

	T_ASSERT (serializerType);

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

	T_FATAL_ASSERT(serializer);
	object = serializer->readObject();

	stream->close();
	return object;
}

bool Instance::setObject(const ISerializable* object)
{
	T_ASSERT (m_providerInstance);

	if (!object)
		return false;

	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	const TypeInfo* serializerType = 0;

	Ref< IStream > stream = m_providerInstance->writeObject(type_name(object), serializerType);
	if (!stream)
		return false;

	T_ASSERT (serializerType);

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
	return result;
}

uint32_t Instance::getDataNames(std::vector< std::wstring >& dataNames) const
{
	T_ASSERT (m_providerInstance);
	return m_providerInstance->getDataNames(dataNames);
}

bool Instance::removeAllData()
{
	T_ASSERT (m_providerInstance);
	return m_providerInstance->removeAllData();
}

Ref< IStream > Instance::readData(const std::wstring& dataName)
{
	T_ASSERT (m_providerInstance);

	Ref< IStream > stream = m_providerInstance->readData(dataName);
	if (!stream)
		return 0;

	return new BufferedStream(stream);
}

Ref< IStream > Instance::writeData(const std::wstring& dataName)
{
	T_ASSERT (m_providerInstance);
	return m_providerInstance->writeData(dataName);
}

Ref< Group > Instance::getParent() const
{
	return m_parent;
}

	}
}
