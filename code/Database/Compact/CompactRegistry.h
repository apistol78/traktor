/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor::db
{

class CompactBlockEntry;
class CompactGroupEntry;
class CompactInstanceEntry;
class CompactRegistry;

/*! Compact registry block entry.
 * \ingroup Database
 */
class CompactBlockEntry : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setBlockId(uint32_t blockId) { m_blockId = blockId; }

	uint32_t getBlockId() const { return m_blockId; }

	virtual void serialize(ISerializer& s) override final;

private:
	uint32_t m_blockId = 0;
};

/*! Compact registry group entry.
 * \ingroup Database
 */
class CompactGroupEntry : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setName(const std::wstring& name) { m_name = name; }

	const std::wstring& getName() const { return m_name; }

	void addChildGroup(CompactGroupEntry* childGroup) { m_childGroups.push_back(childGroup); }

	void addChildInstance(CompactInstanceEntry* childInstance) { return m_childInstances.push_back(childInstance); }

	const RefArray< CompactGroupEntry >& getChildGroups() const { return m_childGroups; }

	const RefArray< CompactInstanceEntry >& getChildInstances() const { return m_childInstances; }

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_name;
	RefArray< CompactGroupEntry > m_childGroups;
	RefArray< CompactInstanceEntry > m_childInstances;
};

/*! Compact registry instance entry.
 * \ingroup Database
 */
class CompactInstanceEntry : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setName(const std::wstring& name) { m_name = name; }

	const std::wstring& getName() const { return m_name; }

	void setGuid(const Guid& guid) { m_guid = guid; }

	const Guid& getGuid() const { return m_guid; }

	void setPrimaryTypeName(const std::wstring& primaryTypeName) { m_primaryTypeName = primaryTypeName; }

	const std::wstring& getPrimaryTypeName() const { return m_primaryTypeName; }

	void setObjectBlock(CompactBlockEntry* objectBlock) { m_objectBlock = objectBlock; }

	CompactBlockEntry* getObjectBlock() const { return m_objectBlock; }

	SmallMap< std::wstring, Ref< CompactBlockEntry > >& getDataBlocks() { return m_dataBlocks; }

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_name;
	Guid m_guid;
	std::wstring m_primaryTypeName;
	Ref< CompactBlockEntry > m_objectBlock;
	SmallMap< std::wstring, Ref< CompactBlockEntry > > m_dataBlocks;
};

/*! Compact registry.
 * \ingroup Database
 */
class CompactRegistry : public ISerializable
{
	T_RTTI_CLASS;

public:
	CompactGroupEntry* createGroupEntry();

	CompactInstanceEntry* createInstanceEntry();

	CompactBlockEntry* createBlockEntry();

	bool removeGroup(CompactGroupEntry* groupEntry);

	bool removeInstance(CompactInstanceEntry* instanceEntry);

	bool removeBlock(CompactBlockEntry* blockEntry);

	void setRootGroup(CompactGroupEntry* rootGroup) { m_rootGroup = rootGroup; }

	CompactGroupEntry* getRootGroup() { return m_rootGroup; }

	const RefArray< CompactGroupEntry >& getGroupEntries() const { return m_groupEntries; }

	const RefArray< CompactInstanceEntry >& getInstanceEntries() const { return m_instanceEntries; }

	const RefArray< CompactBlockEntry >& getBlockEntries() const { return m_blockEntries; }

	virtual void serialize(ISerializer& s) override final;

private:
	Ref< CompactGroupEntry > m_rootGroup;
	RefArray< CompactGroupEntry > m_groupEntries;
	RefArray< CompactInstanceEntry > m_instanceEntries;
	RefArray< CompactBlockEntry > m_blockEntries;
};

}
