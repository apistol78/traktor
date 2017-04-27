/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberStl.h"
#include "Editor/PipelineDependency.h"
#include "Editor/Pipeline/PipelineDependencySet.h"

namespace traktor
{
	namespace editor
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.PipelineDependencySet", 0, PipelineDependencySet, IPipelineDependencySet)

uint32_t PipelineDependencySet::add(PipelineDependency* dependency)
{
	uint32_t index = m_dependencies.size();
	m_dependencies.push_back(dependency);
	return index;
}

uint32_t PipelineDependencySet::add(const Guid& dependencyGuid, PipelineDependency* dependency)
{
	uint32_t index = add(dependency);
	m_indices[dependencyGuid] = index;
	return index;
}

PipelineDependency* PipelineDependencySet::get(uint32_t index) const
{
	return m_dependencies[index];
}

uint32_t PipelineDependencySet::get(const Guid& dependencyGuid) const
{
	std::map< Guid, uint32_t >::const_iterator i = m_indices.find(dependencyGuid);
	if (i != m_indices.end())
		return i->second;
	else
		return DiInvalid;
}

uint32_t PipelineDependencySet::size() const
{
	return m_dependencies.size();
}

void PipelineDependencySet::dump()
{
	for (uint32_t i = 0; i < uint32_t(m_dependencies.size()); ++i)
	{
		log::info << L"dependency[" << i << L"]:" << Endl;
		log::info << IncreaseIndent;
		m_dependencies[i]->dump();
		log::info << DecreaseIndent;
	}
}

void PipelineDependencySet::serialize(ISerializer& s)
{
	s >> MemberRefArray< PipelineDependency >(L"dependencies", m_dependencies);
	s >> MemberStlMap< Guid, uint32_t >(L"indices", m_indices);
}

	}
}
