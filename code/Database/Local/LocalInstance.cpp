#include <cstring>
#include <algorithm>
#include "Database/Local/LocalInstance.h"
#include "Database/Local/LocalInstanceMeta.h"
#include "Database/Local/Context.h"
#include "Database/Local/Transaction.h"
#include "Database/Local/ActionSetGuid.h"
#include "Database/Local/ActionSetName.h"
#include "Database/Local/ActionRemove.h"
#include "Database/Local/ActionRemoveAllData.h"
#include "Database/Local/ActionWriteObject.h"
#include "Database/Local/ActionWriteData.h"
#include "Database/Local/PhysicalAccess.h"
#include "Xml/XmlSerializer.h"
#include "Xml/XmlDeserializer.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Serialization/BinarySerializer.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.LocalInstance", LocalInstance, IProviderInstance)

LocalInstance::LocalInstance(Context* context, const Path& instancePath)
:	m_context(context)
,	m_instancePath(instancePath)
{
}

bool LocalInstance::internalCreateNew(const Guid& instanceGuid)
{
	m_transaction = new Transaction();
	if (!m_transaction->create(instanceGuid))
		return false;

	m_transactionName.clear();

	m_transaction->add(new ActionSetGuid(
		m_instancePath,
		instanceGuid,
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

	m_transaction = new Transaction();
	if (!m_transaction->create(getGuid()))
	{
		m_transaction = 0;
		return false;
	}

	m_transactionName.clear();

	return true;
}

bool LocalInstance::commitTransaction()
{
	if (!m_transaction)
	{
		log::error << L"commitTransaction failed; no pending transaction" << Endl;
		return false;
	}
	if (!m_transaction->commit(m_context))
	{
		log::error << L"commitTransaction failed; commit failed" << Endl;
		return false;
	}
	if (!m_transactionName.empty())
		m_instancePath = m_instancePath = m_instancePath.getPathOnly() + L"/" + m_transactionName;
	return true;
}

bool LocalInstance::closeTransaction()
{
	if (!m_transaction)
		return false;
	
	m_transaction->destroy();
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

	if (getName() == name)
		return true;

	Ref< ActionSetName > action = new ActionSetName(m_instancePath, name);
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

	m_transaction->add(new ActionSetGuid(
		m_instancePath, 
		guid,
		false
	));

	return true;
}

bool LocalInstance::getLastModifyDate(DateTime& outModifyDate) const
{
	Ref< File > instanceObjectFile = FileSystem::getInstance().get(getInstanceObjectPath(m_instancePath));
	Ref< File > instanceMetaFile = FileSystem::getInstance().get(getInstanceMetaPath(m_instancePath));
	if (instanceObjectFile && instanceMetaFile)
	{
		DateTime objectTime = instanceObjectFile->getLastWriteTime();
		DateTime metaTime = instanceMetaFile->getLastWriteTime();
		outModifyDate = DateTime(max< uint64_t >(objectTime, metaTime));
		return true;
	}
	else
		return false;
}

bool LocalInstance::remove()
{
	if (!m_transaction)
		return false;

	m_transaction->add(new ActionRemove(
		m_instancePath
	));

	return true;
}

Ref< IStream > LocalInstance::readObject(const TypeInfo*& outSerializerType) const
{
	Path instanceObjectPath = getInstanceObjectPath(m_instancePath);

	Ref< IStream > objectStream = FileSystem::getInstance().open(instanceObjectPath, File::FmRead);
	if (!objectStream)
		return 0;

	uint8_t head[5];
	if (objectStream->read(head, sizeof(head)) != sizeof(head))
	{
		objectStream->close();
		return 0;
	}

	objectStream->seek(IStream::SeekSet, 0);

	if (std::memcmp(head, "<?xml", sizeof(head)) == 0)
		outSerializerType = &type_of< xml::XmlDeserializer >();
	else
		outSerializerType = &type_of< BinarySerializer >();

	return objectStream;
}

Ref< IStream > LocalInstance::writeObject(const std::wstring& primaryTypeName, const TypeInfo*& outSerializerType)
{
	if (!m_transaction)
		return 0;

	if (!m_context->preferBinary())
		outSerializerType = &type_of< xml::XmlSerializer >();
	else
		outSerializerType = &type_of< BinarySerializer >();

	Ref< ActionWriteObject > action = new ActionWriteObject(
		m_instancePath,
		primaryTypeName
	);
	m_transaction->add(action);

	return action->getStream();
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

bool LocalInstance::removeAllData()
{
	if (!m_transaction)
		return false;

	Ref< ActionRemoveAllData > action = new ActionRemoveAllData(m_instancePath);
	m_transaction->add(action);

	return true;
}

Ref< IStream > LocalInstance::readData(const std::wstring& dataName) const
{
	Path instanceDataPath = getInstanceDataPath(m_instancePath, dataName);
	return FileSystem::getInstance().open(instanceDataPath, File::FmRead);
}

Ref< IStream > LocalInstance::writeData(const std::wstring& dataName)
{
	if (!m_transaction)
		return 0;

	Ref< ActionWriteData > action = new ActionWriteData(m_instancePath, dataName);
	m_transaction->add(action);

	return action->getStream();
}

	}
}
