/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/IStream.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Database/Types.h"
#include "Database/Compact/BlockFile.h"
#include "Database/Compact/CompactContext.h"
#include "Database/Compact/CompactInstance.h"
#include "Database/Compact/CompactRegistry.h"

namespace traktor::db
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.CompactInstance", CompactInstance, IProviderInstance)

CompactInstance::CompactInstance(CompactContext& context)
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
	T_ASSERT(m_instanceEntry);
	return m_instanceEntry->getPrimaryTypeName();
}

bool CompactInstance::openTransaction()
{
	T_ASSERT(m_instanceEntry);
	return true;
}

bool CompactInstance::commitTransaction()
{
	T_ASSERT(m_instanceEntry);
	return true;
}

bool CompactInstance::closeTransaction()
{
	T_ASSERT(m_instanceEntry);
	return true;
}

std::wstring CompactInstance::getName() const
{
	T_ASSERT(m_instanceEntry);
	return m_instanceEntry->getName();
}

bool CompactInstance::setName(const std::wstring& name)
{
	T_ASSERT(m_instanceEntry);
	m_instanceEntry->setName(name);
	return true;
}

Guid CompactInstance::getGuid() const
{
	T_ASSERT(m_instanceEntry);
	return m_instanceEntry->getGuid();
}

bool CompactInstance::setGuid(const Guid& guid)
{
	T_ASSERT(m_instanceEntry);
	m_instanceEntry->setGuid(guid);
	return true;
}

bool CompactInstance::getLastModifyDate(DateTime& outModifyDate) const
{
	return false;
}

uint32_t CompactInstance::getFlags() const
{
	return IfNormal;
}

bool CompactInstance::remove()
{
	T_ASSERT(m_instanceEntry);

	BlockFile* blockFile = m_context.getBlockFile();
	T_ASSERT(blockFile);

	const auto& dataBlocks = m_instanceEntry->getDataBlocks();
	for (auto it : dataBlocks)
	{
		blockFile->freeBlockId(it.second->getBlockId());
		if (!m_context.getRegistry()->removeBlock(it.second))
			return false;
	}

	CompactBlockEntry* objectBlock = m_instanceEntry->getObjectBlock();
	if (objectBlock)
	{
		blockFile->freeBlockId(objectBlock->getBlockId());
		if (!m_context.getRegistry()->removeBlock(objectBlock))
			return false;
	}

	if (!m_context.getRegistry()->removeInstance(m_instanceEntry))
		return false;

	m_instanceEntry = nullptr;
	return true;
}

Ref< IStream > CompactInstance::readObject(const TypeInfo*& outSerializerType) const
{
	T_ASSERT(m_instanceEntry);

	Ref< CompactBlockEntry > objectBlockEntry = m_instanceEntry->getObjectBlock();
	if (!objectBlockEntry)
		return nullptr;

	BlockFile* blockFile = m_context.getBlockFile();
	T_ASSERT(blockFile);

	Ref< IStream > objectStream = blockFile->readBlock(objectBlockEntry->getBlockId());
	if (!objectStream)
		return nullptr;

	outSerializerType = &type_of< BinarySerializer >();
	return objectStream;
}

Ref< IStream > CompactInstance::writeObject(const std::wstring& primaryTypeName, const TypeInfo*& outSerializerType)
{
	T_ASSERT(m_instanceEntry);

	BlockFile* blockFile = m_context.getBlockFile();
	T_ASSERT(blockFile);

	Ref< CompactBlockEntry > objectBlockEntry = m_instanceEntry->getObjectBlock();
	if (!objectBlockEntry)
	{
		uint32_t objectBlockId = blockFile->allocBlockId();

		objectBlockEntry = m_context.getRegistry()->createBlockEntry();
		objectBlockEntry->setBlockId(objectBlockId);

		m_instanceEntry->setObjectBlock(objectBlockEntry);
	}

	Ref< IStream > objectStream = blockFile->writeBlock(objectBlockEntry->getBlockId());
	if (!objectStream)
		return nullptr;

	m_instanceEntry->setPrimaryTypeName(primaryTypeName);

	outSerializerType = &type_of< BinarySerializer >();
	return objectStream;
}

uint32_t CompactInstance::getDataNames(AlignedVector< std::wstring >& outDataNames) const
{
	T_ASSERT(m_instanceEntry);

	const auto& dataBlocks = m_instanceEntry->getDataBlocks();
	for (auto it : dataBlocks)
		outDataNames.push_back(it.first);

	return uint32_t(outDataNames.size());
}

bool CompactInstance::getDataLastWriteTime(const std::wstring& dataName, DateTime& outLastWriteTime) const
{
	return false;
}

bool CompactInstance::removeAllData()
{
	BlockFile* blockFile = m_context.getBlockFile();
	T_ASSERT(blockFile);

	auto& dataBlocks = m_instanceEntry->getDataBlocks();
	for (auto it : dataBlocks)
	{
		blockFile->freeBlockId(it.second->getBlockId());
		if (!m_context.getRegistry()->removeBlock(it.second))
			return false;
	}

	dataBlocks.clear();
	return true;
}

Ref< IStream > CompactInstance::readData(const std::wstring& dataName) const
{
	T_ASSERT(m_instanceEntry);

	const auto& dataBlocks = m_instanceEntry->getDataBlocks();

	const auto it = dataBlocks.find(dataName);
	if (it == dataBlocks.end())
		return nullptr;

	BlockFile* blockFile = m_context.getBlockFile();
	T_ASSERT(blockFile);

	return blockFile->readBlock(it->second->getBlockId());
}

Ref< IStream > CompactInstance::writeData(const std::wstring& dataName)
{
	T_ASSERT(m_instanceEntry);

	BlockFile* blockFile = m_context.getBlockFile();
	T_ASSERT(blockFile);

	auto& dataBlocks = m_instanceEntry->getDataBlocks();
	if (!dataBlocks[dataName])
	{
		dataBlocks[dataName] = m_context.getRegistry()->createBlockEntry();
		dataBlocks[dataName]->setBlockId(blockFile->allocBlockId());
	}

	Ref< IStream > dataStream = blockFile->writeBlock(dataBlocks[dataName]->getBlockId());
	if (!dataStream)
		return nullptr;

	return dataStream;
}

}
