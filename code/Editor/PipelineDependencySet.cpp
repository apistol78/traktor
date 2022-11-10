/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Editor/PipelineDependency.h"
#include "Editor/PipelineDependencySet.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.PipelineDependencySet", 0, PipelineDependencySet, ISerializable)

uint32_t PipelineDependencySet::add(PipelineDependency* dependency)
{
	uint32_t index = (uint32_t)m_dependencies.size();
	m_dependencies.push_back(dependency);
	return index;
}

uint32_t PipelineDependencySet::add(const Guid& dependencyGuid, PipelineDependency* dependency)
{
	uint32_t index = add(dependency);
	m_indices[dependencyGuid] = index;
	return index;
}

PipelineDependency* PipelineDependencySet::get(uint32_t index)
{
	return m_dependencies[index];
}

const PipelineDependency* PipelineDependencySet::get(uint32_t index) const
{
	return m_dependencies[index];
}

uint32_t PipelineDependencySet::get(const Guid& dependencyGuid) const
{
	auto it = m_indices.find(dependencyGuid);
	if (it != m_indices.end())
		return it->second;
	else
		return DiInvalid;
}

uint32_t PipelineDependencySet::size() const
{
	return (uint32_t)m_dependencies.size();
}

void PipelineDependencySet::dump(OutputStream& os) const
{
	for (uint32_t i = 0; i < uint32_t(m_dependencies.size()); ++i)
	{
		os << L"dependency[" << i << L"]:" << Endl;
		os << IncreaseIndent;
		m_dependencies[i]->dump(os);
		os << DecreaseIndent;
	}
}

void PipelineDependencySet::serialize(ISerializer& s)
{
	s >> MemberRefArray< PipelineDependency >(L"dependencies", m_dependencies);
	s >> MemberSmallMap< Guid, uint32_t >(L"indices", m_indices);
}

}
