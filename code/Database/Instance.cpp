#include "Database/Instance.h"
#include "Database/Group.h"
#include "Database/Provider/IProviderInstance.h"
#include "Database/Provider/IProviderBus.h"
#include "Xml/XmlSerializer.h"
#include "Xml/XmlDeserializer.h"
#include "Core/Serialization/Serializable.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Io/Stream.h"
#include "Core/Thread/Acquire.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.Instance", Instance, Object)

Instance::Instance(IProviderBus* providerBus)
:	m_providerBus(providerBus)
,	m_renamed(false)
,	m_removed(false)
{
}

bool Instance::internalCreate(IProviderInstance* providerInstance, Group* parent)
{
	m_providerInstance = providerInstance;
	m_parent = parent;
	return true;
}

void Instance::internalDestroy()
{
	m_providerBus = 0;
	m_providerInstance = 0;
	m_parent = 0;
	Heap::getInstance().invalidateRefs(this);
}

std::wstring Instance::getName() const
{
	T_ASSERT (m_providerInstance);
	return m_providerInstance->getName();
}

std::wstring Instance::getPath() const
{
	T_ASSERT (m_providerInstance);
	return m_parent->getPath() + L"/" + m_providerInstance->getName();
}

Guid Instance::getGuid() const
{
	T_ASSERT (m_providerInstance);
	return m_providerInstance->getGuid();
}

std::wstring Instance::getPrimaryTypeName() const
{
	T_ASSERT (m_providerInstance);
	return m_providerInstance->getPrimaryTypeName();
}

const Type* Instance::getPrimaryType() const
{
	T_ASSERT (m_providerInstance);
	return Type::find(getPrimaryTypeName());
}

bool Instance::checkout()
{
	T_ASSERT (m_providerInstance);

	Acquire< Mutex > __lock__(m_lock);

	if (!m_providerInstance->openTransaction())
		return false;

	m_renamed = false;
	m_removed = false;

	return true;
}

bool Instance::commit(uint32_t flags)
{
	T_ASSERT (m_providerInstance);

	Acquire< Mutex > __lock__(m_lock);

	if ((flags & CfKeepCheckedOut) != 0 && m_removed)
		return false;

	if (!m_providerInstance->commitTransaction())
		return false;

	if ((flags & CfKeepCheckedOut) == 0)
	{
		if (!m_providerInstance->closeTransaction())
			return false;
	}

	if (m_providerBus)
	{
		Guid guid = getGuid();
		m_providerBus->putEvent(PeCommited, guid);
		if (m_removed)
			m_providerBus->putEvent(PeRemoved, guid);
		if (m_renamed)
			m_providerBus->putEvent(PeRenamed, guid);
	}

	if (m_removed)
	{
		if (m_parent)
			m_parent->removeChildInstance(this);
		internalDestroy();
	}

	return true;
}

bool Instance::revert()
{
	T_ASSERT (m_providerInstance);

	Acquire< Mutex > __lock__(m_lock);

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

Serializable* Instance::getObject()
{
	T_ASSERT (m_providerInstance);

	Acquire< Mutex > __lock__(m_lock);
	Ref< Serializable > object;
	const Type* serializerType = 0;

	Ref< Stream > stream = m_providerInstance->readObject(serializerType);
	if (!stream)
		return 0;

	T_ASSERT (serializerType);

	Ref< Serializer > serializer;
	if (serializerType == &type_of< BinarySerializer >())
		serializer = gc_new< BinarySerializer >(stream);
	else if (serializerType == &type_of< xml::XmlDeserializer >())
		serializer = gc_new< xml::XmlDeserializer >(stream);
	else
	{
		stream->close();
		return 0;
	}

	object = serializer->readObject();

	stream->close();
	return object;
}

bool Instance::setObject(const Serializable* object)
{
	T_ASSERT (m_providerInstance);

	if (!object)
		return false;

	Acquire< Mutex > __lock__(m_lock);
	const Type* serializerType = 0;

	Ref< Stream > stream = m_providerInstance->writeObject(type_name(object), serializerType);
	if (!stream)
		return false;

	T_ASSERT (serializerType);

	Ref< Serializer > serializer;
	if (serializerType == &type_of< BinarySerializer >())
		serializer = gc_new< BinarySerializer >(stream);
	else if (serializerType == &type_of< xml::XmlSerializer >())
		serializer = gc_new< xml::XmlSerializer >(stream);
	else
	{
		stream->close();
		return false;
	}

	bool result = serializer->writeObject(object);

	stream->close();
	return result;
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

Group* Instance::getParent() const
{
	return m_parent;
}

	}
}
