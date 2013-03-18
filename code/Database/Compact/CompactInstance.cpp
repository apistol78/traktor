#include "Database/Compact/CompactInstance.h"
#include "Database/Compact/CompactContext.h"
#include "Database/Compact/CompactRegistry.h"
#include "Database/Compact/BlockFile.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Io/IStream.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.CompactInstance", CompactInstance, IProviderInstance)

CompactInstance::CompactInstance(CompactContext* context)
:	m_context(context)
{
}

bool CompactInstance::internalCreate(CompactInstanceEntry* instanceEntry)
{
	m_instanceEntry = instanceEntry;
	return true;
}

std::wstring CompactInstance::getPrimaryTypeName() const
{
	T_ASSERT (m_instanceEntry);
	return m_instanceEntry->getPrimaryTypeName();
}

bool CompactInstance::openTransaction()
{
	T_ASSERT (m_instanceEntry);
	return true;
}

bool CompactInstance::commitTransaction()
{
	T_ASSERT (m_instanceEntry);
	return true;
}

bool CompactInstance::closeTransaction()
{
	T_ASSERT (m_instanceEntry);
	return true;
}

std::wstring CompactInstance::getName() const
{
	T_ASSERT (m_instanceEntry);
	return m_instanceEntry->getName();
}

bool CompactInstance::setName(const std::wstring& name)
{
	T_ASSERT (m_instanceEntry);
	m_instanceEntry->setName(name);
	return true;
}

Guid CompactInstance::getGuid() const
{
	T_ASSERT (m_instanceEntry);
	return m_instanceEntry->getGuid();
}

bool CompactInstance::setGuid(const Guid& guid)
{
	T_ASSERT (m_instanceEntry);
	m_instanceEntry->setGuid(guid);
	return true;
}

bool CompactInstance::getLastModifyDate(DateTime& outModifyDate) const
{
	return false;
}

bool CompactInstance::remove()
{
	T_ASSERT (m_instanceEntry);
	
	Ref< BlockFile > blockFile = m_context->getBlockFile();
	T_ASSERT (blockFile);

	const std::map< std::wstring, Ref< CompactBlockEntry > >& dataBlocks = m_instanceEntry->getDataBlocks();
	for (std::map< std::wstring, Ref< CompactBlockEntry > >::const_iterator i = dataBlocks.begin(); i != dataBlocks.end(); ++i)
	{
		blockFile->freeBlockId(i->second->getBlockId());
		if (!m_context->getRegistry()->removeBlock(i->second))
			return false;
	}

	Ref< CompactBlockEntry > objectBlock = m_instanceEntry->getObjectBlock();
	if (objectBlock)
	{
		blockFile->freeBlockId(objectBlock->getBlockId());
		if (!m_context->getRegistry()->removeBlock(objectBlock))
			return false;
	}

	if (!m_context->getRegistry()->removeInstance(m_instanceEntry))
		return false;

	m_instanceEntry = 0;
	return true;
}

Ref< IStream > CompactInstance::readObject(const TypeInfo*& outSerializerType) const
{
	T_ASSERT (m_instanceEntry);

	Ref< CompactBlockEntry > objectBlockEntry = m_instanceEntry->getObjectBlock();
	if (!objectBlockEntry)
		return 0;

	Ref< BlockFile > blockFile = m_context->getBlockFile();
	T_ASSERT (blockFile);

	Ref< IStream > objectStream = blockFile->readBlock(objectBlockEntry->getBlockId());
	if (!objectStream)
		return 0;

	outSerializerType = &type_of< BinarySerializer >();
	return objectStream;
}

Ref< IStream > CompactInstance::writeObject(const std::wstring& primaryTypeName, const TypeInfo*& outSerializerType)
{
	T_ASSERT (m_instanceEntry);

	Ref< BlockFile > blockFile = m_context->getBlockFile();
	T_ASSERT (blockFile);

	Ref< CompactBlockEntry > objectBlockEntry = m_instanceEntry->getObjectBlock();
	if (!objectBlockEntry)
	{
		uint32_t objectBlockId = blockFile->allocBlockId();

		objectBlockEntry = m_context->getRegistry()->createBlockEntry();
		objectBlockEntry->setBlockId(objectBlockId);

		m_instanceEntry->setObjectBlock(objectBlockEntry);
	}

	Ref< IStream > objectStream = blockFile->writeBlock(objectBlockEntry->getBlockId());
	if (!objectStream)
		return 0;

	m_instanceEntry->setPrimaryTypeName(primaryTypeName);

	outSerializerType = &type_of< BinarySerializer >();
	return objectStream;
}

uint32_t CompactInstance::getDataNames(std::vector< std::wstring >& outDataNames) const
{
	T_ASSERT (m_instanceEntry);

	const std::map< std::wstring, Ref< CompactBlockEntry > >& dataBlocks = m_instanceEntry->getDataBlocks();
	for (std::map< std::wstring, Ref< CompactBlockEntry > >::const_iterator i = dataBlocks.begin(); i != dataBlocks.end(); ++i)
		outDataNames.push_back(i->first);
	
	return uint32_t(outDataNames.size());
}

bool CompactInstance::removeAllData()
{
	std::map< std::wstring, Ref< CompactBlockEntry > >& dataBlocks = m_instanceEntry->getDataBlocks();
	dataBlocks.clear();
	return true;
}

Ref< IStream > CompactInstance::readData(const std::wstring& dataName) const
{
	T_ASSERT (m_instanceEntry);

	const std::map< std::wstring, Ref< CompactBlockEntry > >& dataBlocks = m_instanceEntry->getDataBlocks();

	std::map< std::wstring, Ref< CompactBlockEntry > >::const_iterator i = dataBlocks.find(dataName);
	if (i == dataBlocks.end())
		return 0;

	Ref< BlockFile > blockFile = m_context->getBlockFile();
	T_ASSERT (blockFile);

	return blockFile->readBlock(i->second->getBlockId());
}

Ref< IStream > CompactInstance::writeData(const std::wstring& dataName)
{
	T_ASSERT (m_instanceEntry);

	Ref< BlockFile > blockFile = m_context->getBlockFile();
	T_ASSERT (blockFile);

	std::map< std::wstring, Ref< CompactBlockEntry > >& dataBlocks = m_instanceEntry->getDataBlocks();
	if (!dataBlocks[dataName])
	{
		dataBlocks[dataName] = m_context->getRegistry()->createBlockEntry();
		dataBlocks[dataName]->setBlockId(blockFile->allocBlockId());
	}

	Ref< IStream > dataStream = blockFile->writeBlock(dataBlocks[dataName]->getBlockId());
	if (!dataStream)
		return 0;

	return dataStream;
}

	}
}
