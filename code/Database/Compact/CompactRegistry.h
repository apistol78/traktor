#ifndef traktor_db_CompactRegistry_H
#define traktor_db_CompactRegistry_H

#include <list>
#include <map>
#include "Core/Guid.h"
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace db
	{

class CompactBlockEntry;
class CompactGroupEntry;
class CompactInstanceEntry;
class CompactRegistry;

/*! \brief Compact registry block entry.
 * \ingroup Database
 */
class CompactBlockEntry : public ISerializable
{
	T_RTTI_CLASS;

public:
	CompactBlockEntry()
	:	m_blockId(0)
	{
	}

	inline void setBlockId(uint32_t blockId) { m_blockId = blockId; }

	inline uint32_t getBlockId() const { return m_blockId; }

	virtual bool serialize(ISerializer& s);

private:
	uint32_t m_blockId;
};

/*! \brief Compact registry group entry.
 * \ingroup Database
 */
class CompactGroupEntry : public ISerializable
{
	T_RTTI_CLASS;

public:
	inline void setName(const std::wstring& name) { m_name = name; }

	inline const std::wstring& getName() const { return m_name; }

	inline void addChildGroup(CompactGroupEntry* childGroup) { m_childGroups.push_back(childGroup); }

	inline void addChildInstance(CompactInstanceEntry* childInstance) { return m_childInstances.push_back(childInstance); }

	inline const RefArray< CompactGroupEntry >& getChildGroups() const { return m_childGroups; }

	inline const RefArray< CompactInstanceEntry >& getChildInstances() const { return m_childInstances; }

	virtual bool serialize(ISerializer& s);

private:
	std::wstring m_name;
	RefArray< CompactGroupEntry > m_childGroups;
	RefArray< CompactInstanceEntry > m_childInstances;
};

/*! \brief Compact registry instance entry.
 * \ingroup Database
 */
class CompactInstanceEntry : public ISerializable
{
	T_RTTI_CLASS;

public:
	inline void setName(const std::wstring& name) { m_name = name; }

	inline const std::wstring& getName() const { return m_name; }

	inline void setGuid(const Guid& guid) { m_guid = guid; }

	inline const Guid& getGuid() const { return m_guid; }

	inline void setPrimaryTypeName(const std::wstring& primaryTypeName) { m_primaryTypeName = primaryTypeName; }

	inline const std::wstring& getPrimaryTypeName() const { return m_primaryTypeName; }

	inline void setObjectBlock(CompactBlockEntry* objectBlock) { m_objectBlock = objectBlock; }

	inline CompactBlockEntry* getObjectBlock() const { return m_objectBlock; }

	inline std::map< std::wstring, Ref< CompactBlockEntry > >& getDataBlocks() { return m_dataBlocks; }

	virtual bool serialize(ISerializer& s);

private:
	std::wstring m_name;
	Guid m_guid;
	std::wstring m_primaryTypeName;
	Ref< CompactBlockEntry > m_objectBlock;
	std::map< std::wstring, Ref< CompactBlockEntry > > m_dataBlocks;
};

/*! \brief Compact registry.
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

	inline void setRootGroup(CompactGroupEntry* rootGroup) { m_rootGroup = rootGroup; }

	inline CompactGroupEntry* getRootGroup() { return m_rootGroup; }

	inline const RefArray< CompactGroupEntry >& getGroupEntries() const { return m_groupEntries; }

	inline const RefArray< CompactInstanceEntry >& getInstanceEntries() const { return m_instanceEntries; }

	inline const RefArray< CompactBlockEntry >& getBlockEntries() const { return m_blockEntries; }

	virtual bool serialize(ISerializer& s);

private:
	Ref< CompactGroupEntry > m_rootGroup;
	RefArray< CompactGroupEntry > m_groupEntries;
	RefArray< CompactInstanceEntry > m_instanceEntries;
	RefArray< CompactBlockEntry > m_blockEntries;
};

	}
}

#endif	// traktor_db_CompactRegistry_H
