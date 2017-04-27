/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Compact/CompactGroup.h"
#include "Database/Compact/CompactInstance.h"
#include "Database/Compact/CompactContext.h"
#include "Database/Compact/CompactRegistry.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.CompactGroup", CompactGroup, IProviderGroup)

CompactGroup::CompactGroup(CompactContext* context)
:	m_context(context)
{
}

bool CompactGroup::internalCreate(CompactGroupEntry* groupEntry)
{
	m_groupEntry = groupEntry;

	const RefArray< CompactGroupEntry >& childGroupEntries = groupEntry->getChildGroups();
	for (RefArray< CompactGroupEntry >::const_iterator i = childGroupEntries.begin(); i != childGroupEntries.end(); ++i)
	{
		Ref< CompactGroup > childGroup = new CompactGroup(m_context);
		if (!childGroup->internalCreate(*i))
			return false;
		m_childGroups.push_back(childGroup);
	}

	const RefArray< CompactInstanceEntry >& childInstanceEntries = groupEntry->getChildInstances();
	for (RefArray< CompactInstanceEntry >::const_iterator i = childInstanceEntries.begin(); i != childInstanceEntries.end(); ++i)
	{
		Ref< CompactInstance > childInstance = new CompactInstance(m_context);
		if (!childInstance->internalCreate(*i))
			return false;
		m_childInstances.push_back(childInstance);
	}

	return true;
}

std::wstring CompactGroup::getName() const
{
	T_ASSERT (m_groupEntry);
	return m_groupEntry->getName();
}

bool CompactGroup::rename(const std::wstring& name)
{
	T_ASSERT (m_groupEntry);
	m_groupEntry->setName(name);
	return true;
}

bool CompactGroup::remove()
{
	T_ASSERT (m_groupEntry);
	if (!m_context->getRegistry()->removeGroup(m_groupEntry))
		return false;
	m_groupEntry = 0;
	return true;
}

Ref< IProviderGroup > CompactGroup::createGroup(const std::wstring& groupName)
{
	T_ASSERT (m_groupEntry);

	Ref< CompactGroupEntry > childGroupEntry = m_context->getRegistry()->createGroupEntry();
	if (!childGroupEntry)
		return 0;

	childGroupEntry->setName(groupName);

	Ref< CompactGroup > childGroup = new CompactGroup(m_context);
	if (!childGroup->internalCreate(childGroupEntry))
		return 0;

	m_childGroups.push_back(childGroup);
	m_groupEntry->addChildGroup(childGroupEntry);

	return childGroup;
}

Ref< IProviderInstance > CompactGroup::createInstance(const std::wstring& instanceName, const Guid& instanceGuid)
{
	T_ASSERT (m_groupEntry);

	Ref< CompactInstanceEntry > childInstanceEntry = m_context->getRegistry()->createInstanceEntry();
	if (!childInstanceEntry)
		return 0;

	childInstanceEntry->setName(instanceName);
	childInstanceEntry->setGuid(instanceGuid);

	Ref< CompactInstance > childInstance = new CompactInstance(m_context);
	if (!childInstance->internalCreate(childInstanceEntry))
		return 0;

	m_childInstances.push_back(childInstance);
	m_groupEntry->addChildInstance(childInstanceEntry);

	return childInstance;
}

bool CompactGroup::getChildren(RefArray< IProviderGroup >& outChildGroups, RefArray< IProviderInstance >& outChildInstances)
{
	outChildGroups.reserve(m_childGroups.size());
	for (RefArray< CompactGroup >::iterator i = m_childGroups.begin(); i != m_childGroups.end(); ++i)
		outChildGroups.push_back(*i);

	outChildInstances.reserve(m_childInstances.size());
	for (RefArray< CompactInstance >::iterator i = m_childInstances.begin(); i != m_childInstances.end(); ++i)
		outChildInstances.push_back(*i);

	return true;
}

	}
}
