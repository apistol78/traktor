/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Serialization/ISerializer.h>
#include <Core/Serialization/Member.h>
#include <Core/Serialization/MemberRefArray.h>
#include "SolutionBuilderLIB/Aggregation.h"
#include "SolutionBuilderLIB/AggregationItem.h"
#include "SolutionBuilderLIB/Dependency.h"

using namespace traktor;

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"Aggregation", 1, Aggregation, ISerializable)

Aggregation::Aggregation()
:	m_enable(true)
{
}

void Aggregation::setEnable(bool enable)
{
	m_enable = enable;
}

bool Aggregation::getEnable() const
{
	return m_enable;
}

void Aggregation::setName(const std::wstring& name)
{
	m_name = name;
}

const std::wstring& Aggregation::getName() const
{
	return m_name;
}

void Aggregation::addItem(AggregationItem* item)
{
	m_items.push_back(item);
}

void Aggregation::removeItem(AggregationItem* item)
{
	m_items.remove(item);
}

const RefArray< AggregationItem >& Aggregation::getItems() const
{
	return m_items;
}

void Aggregation::addDependency(Dependency* dependency)
{
	m_dependencies.push_back(dependency);
}

void Aggregation::removeDependency(Dependency* dependency)
{
	m_dependencies.remove(dependency);
}

void Aggregation::setDependencies(const RefArray< Dependency >& dependencies)
{
	m_dependencies = dependencies;
}
	
const RefArray< Dependency >& Aggregation::getDependencies() const
{
	return m_dependencies;
}

void Aggregation::serialize(ISerializer& s)
{
	s >> Member< bool >(L"enable", m_enable);
	s >> Member< std::wstring >(L"name", m_name);
	
	if (s.getVersion() >= 1)
		s >> MemberRefArray< AggregationItem >(L"items", m_items);

	s >> MemberRefArray< Dependency >(L"dependencies", m_dependencies);
}
