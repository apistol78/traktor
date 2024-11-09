/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Misc/String.h"
#include "Editor/App/MRU.h"

namespace traktor::editor
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.editor.MRU", 1, MRU, ISerializable)

void MRU::usedFile(const Path& filePath)
{
	// Always handle absolute paths.
	const std::wstring fullPath = FileSystem::getInstance().getAbsolutePath(filePath).getPathNameOS();

	// Remove existing entry; we will re-add below as most recent.
	auto it = std::find_if(m_filePaths.begin(), m_filePaths.end(), [&](const std::wstring& str) {
		return compareIgnoreCase(str, fullPath) == 0;
	});
	if (it != m_filePaths.end())
		m_filePaths.erase(it);

	m_filePaths.insert(m_filePaths.begin(), fullPath);
	if (m_filePaths.size() > 8)
		m_filePaths.pop_back();
}

AlignedVector< Path > MRU::getUsedFiles() const
{
	AlignedVector< Path > filePaths;
	for (const auto& filePath : m_filePaths)
		filePaths.push_back(Path(filePath));
	return filePaths;
}

Path MRU::getMostRecentlyUseFile() const
{
	return !m_filePaths.empty() ? m_filePaths.front() : Path();
}

void MRU::usedInstance(const Guid& instance)
{
	// Remove existing entry; we will re-add below as most recent.
	auto it = std::find(m_instances.begin(), m_instances.end(), instance);
	if (it != m_instances.end())
		m_instances.erase(it);

	m_instances.insert(m_instances.begin(), instance);
	if (m_instances.size() > 8)
		m_instances.pop_back();
}

AlignedVector< Guid > MRU::getUsedInstances() const
{
	return m_instances;
}

void MRU::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< std::wstring >(L"filePaths", m_filePaths);
	if (s.getVersion< MRU >() >= 1)
		s >> MemberAlignedVector< Guid >(L"instances", m_instances);
}

}
