#include <algorithm>
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAggregate.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Database/Compact/CompactRegistry.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.CompactBlockEntry", CompactBlockEntry, ISerializable)

bool CompactBlockEntry::serialize(ISerializer& s)
{
	return s >> Member< uint32_t >(L"blockId", m_blockId);
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.CompactGroupEntry", CompactGroupEntry, ISerializable)

bool CompactGroupEntry::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> MemberRefArray< CompactGroupEntry >(L"childGroups", m_childGroups);
	s >> MemberRefArray< CompactInstanceEntry >(L"childInstances", m_childInstances);
	return true;
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.CompactInstanceEntry", CompactInstanceEntry, ISerializable)

bool CompactInstanceEntry::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< Guid >(L"guid", m_guid);
	s >> Member< std::wstring >(L"primaryTypeName", m_primaryTypeName);
	s >> MemberRef< CompactBlockEntry >(L"objectBlock", m_objectBlock);
	s >> MemberStlMap
		< 
			std::wstring,
			Ref< CompactBlockEntry >,
			MemberStlPair
			<
				std::wstring,
				Ref< CompactBlockEntry >,
				Member< std::wstring >,
				MemberRef< CompactBlockEntry >
			>
		>
		(L"dataBlocks", m_dataBlocks);
	return true;
}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.CompactRegistry", CompactRegistry, ISerializable)

CompactGroupEntry* CompactRegistry::createGroupEntry()
{
	Ref< CompactGroupEntry > groupEntry = new CompactGroupEntry();
	m_groupEntries.push_back(groupEntry);
	return groupEntry;
}

CompactInstanceEntry* CompactRegistry::createInstanceEntry()
{
	Ref< CompactInstanceEntry > instanceEntry = new CompactInstanceEntry();
	m_instanceEntries.push_back(instanceEntry);
	return instanceEntry;
}

CompactBlockEntry* CompactRegistry::createBlockEntry()
{
	Ref< CompactBlockEntry > blockEntry = new CompactBlockEntry();
	m_blockEntries.push_back(blockEntry);
	return blockEntry;
}

bool CompactRegistry::removeGroup(CompactGroupEntry* groupEntry)
{
	RefArray< CompactGroupEntry >::iterator i = std::find(m_groupEntries.begin(), m_groupEntries.end(), groupEntry);
	if (i == m_groupEntries.end())
		return false;

	m_groupEntries.erase(i);
	//Heap::getInstance().invalidateRefs(groupEntry);

	return true;
}

bool CompactRegistry::removeInstance(CompactInstanceEntry* instanceEntry)
{
	RefArray< CompactInstanceEntry >::iterator i = std::find(m_instanceEntries.begin(), m_instanceEntries.end(), instanceEntry);
	if (i == m_instanceEntries.end())
		return false;

	m_instanceEntries.erase(i);
	//Heap::getInstance().invalidateRefs(instanceEntry);

	return true;
}

bool CompactRegistry::removeBlock(CompactBlockEntry* blockEntry)
{
	RefArray< CompactBlockEntry >::iterator i = std::find(m_blockEntries.begin(), m_blockEntries.end(), blockEntry);
	if (i == m_blockEntries.end())
		return false;

	m_blockEntries.erase(i);
	//Heap::getInstance().invalidateRefs(blockEntry);

	return true;
}

bool CompactRegistry::serialize(ISerializer& s)
{
	s >> MemberRef< CompactGroupEntry >(L"rootGroup", m_rootGroup);
	s >> MemberRefArray< CompactGroupEntry >(L"groupEntries", m_groupEntries);
	s >> MemberRefArray< CompactInstanceEntry >(L"instanceEntries", m_instanceEntries);
	s >> MemberRefArray< CompactBlockEntry >(L"blockEntries", m_blockEntries);
	return true;
}

	}
}
