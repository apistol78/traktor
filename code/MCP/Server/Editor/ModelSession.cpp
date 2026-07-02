/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "MCP/Server/Editor/ModelSession.h"

#include "Model/Model.h"

#include <algorithm>

namespace traktor::mcp
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mcp.ModelSession", ModelSession, Object)

int32_t ModelSession::open(model::Model* model, SourceKind sourceKind, const std::wstring& filePath, const Guid& meshAssetGuid, const std::wstring& importFilter)
{
	const int32_t handle = m_nextHandle++;
	Entry& e = m_entries[handle];
	e.model = model;
	e.sourceKind = sourceKind;
	e.filePath = filePath;
	e.meshAssetGuid = meshAssetGuid;
	e.importFilter = importFilter;
	return handle;
}

model::Model* ModelSession::get(int32_t handle) const
{
	auto it = m_entries.find(handle);
	return it != m_entries.end() ? it->second.model : nullptr;
}

const ModelSession::Entry* ModelSession::entry(int32_t handle) const
{
	auto it = m_entries.find(handle);
	return it != m_entries.end() ? &it->second : nullptr;
}

bool ModelSession::close(int32_t handle)
{
	auto it = m_entries.find(handle);
	if (it == m_entries.end())
		return false;
	m_entries.erase(it);
	return true;
}

int32_t ModelSession::closeAll()
{
	const int32_t n = (int32_t)m_entries.size();
	m_entries.clear();
	return n;
}

std::vector< int32_t > ModelSession::handles() const
{
	std::vector< int32_t > result;
	result.reserve(m_entries.size());
	for (const auto& it : m_entries)
		result.push_back(it.first);
	std::sort(result.begin(), result.end());
	return result;
}

}
