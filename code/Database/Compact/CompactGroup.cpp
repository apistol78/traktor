/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Database/Types.h"
#include "Database/Compact/CompactGroup.h"
#include "Database/Compact/CompactInstance.h"
#include "Database/Compact/CompactContext.h"
#include "Database/Compact/CompactRegistry.h"

namespace traktor::db
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.CompactGroup", CompactGroup, IProviderGroup)

CompactGroup::CompactGroup(CompactContext& context)
:	m_context(context)
{
}

bool CompactGroup::internalCreate(CompactGroupEntry* groupEntry)
{
	m_groupEntry = groupEntry;

	const RefArray< CompactGroupEntry >& childGroupEntries = groupEntry->getChildGroups();
	for (auto childGroupEntry : childGroupEntries)
	{
		Ref< CompactGroup > childGroup = new CompactGroup(m_context);
		if (!childGroup->internalCreate(childGroupEntry))
			return false;
		m_childGroups.push_back(childGroup);
	}

	const RefArray< CompactInstanceEntry >& childInstanceEntries = groupEntry->getChildInstances();
	for (auto childInstanceEntry : childInstanceEntries)
	{
		Ref< CompactInstance > childInstance = new CompactInstance(m_context);
		if (!childInstance->internalCreate(childInstanceEntry))
			return false;
		m_childInstances.push_back(childInstance);
	}

	return true;
}

std::wstring CompactGroup::getName() const
{
	T_ASSERT(m_groupEntry);
	return m_groupEntry->getName();
}

uint32_t CompactGroup::getFlags() const
{
	return GfNormal;
}

bool CompactGroup::rename(const std::wstring& name)
{
	T_ASSERT(m_groupEntry);
	m_groupEntry->setName(name);
	return true;
}

bool CompactGroup::remove()
{
	T_ASSERT(m_groupEntry);
	if (!m_context.getRegistry()->removeGroup(m_groupEntry))
		return false;
	m_groupEntry = nullptr;
	return true;
}

Ref< IProviderGroup > CompactGroup::createGroup(const std::wstring& groupName)
{
	T_ASSERT(m_groupEntry);

	Ref< CompactGroupEntry > childGroupEntry = m_context.getRegistry()->createGroupEntry();
	if (!childGroupEntry)
		return nullptr;

	childGroupEntry->setName(groupName);

	Ref< CompactGroup > childGroup = new CompactGroup(m_context);
	if (!childGroup->internalCreate(childGroupEntry))
		return nullptr;

	m_childGroups.push_back(childGroup);
	m_groupEntry->addChildGroup(childGroupEntry);

	return childGroup;
}

Ref< IProviderInstance > CompactGroup::createInstance(const std::wstring& instanceName, const Guid& instanceGuid)
{
	T_ASSERT(m_groupEntry);

	Ref< CompactInstanceEntry > childInstanceEntry = m_context.getRegistry()->createInstanceEntry();
	if (!childInstanceEntry)
		return nullptr;

	childInstanceEntry->setName(instanceName);
	childInstanceEntry->setGuid(instanceGuid);

	Ref< CompactInstance > childInstance = new CompactInstance(m_context);
	if (!childInstance->internalCreate(childInstanceEntry))
		return nullptr;

	m_childInstances.push_back(childInstance);
	m_groupEntry->addChildInstance(childInstanceEntry);

	return childInstance;
}

bool CompactGroup::getChildren(RefArray< IProviderGroup >& outChildGroups, RefArray< IProviderInstance >& outChildInstances)
{
	outChildGroups.reserve(m_childGroups.size());
	for (auto childGroup : m_childGroups)
		outChildGroups.push_back(childGroup);

	outChildInstances.reserve(m_childInstances.size());
	for (auto childInstance : m_childInstances)
		outChildInstances.push_back(childInstance);

	return true;
}

}
