/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAggregate.h"
#include "Core/Serialization/MemberStl.h"
#include "SolutionBuilder/Editor/App/MRU.h"

namespace traktor::sb
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sb.MRU", 0, MRU, ISerializable)

void MRU::usedFile(const Path& filePath)
{
	// Always handle absolute paths.
	const std::wstring fullPath = FileSystem::getInstance().getAbsolutePath(filePath)
#if defined(_WIN32)
		.getPathName();
#else
		.getPathNameNoVolume();
#endif

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

bool MRU::getUsedFiles(std::vector< Path >& outFilePaths) const
{
	for (const auto& filePath : m_filePaths)
		outFilePaths.push_back(Path(filePath));
	return true;
}

void MRU::serialize(ISerializer& s)
{
	s >> MemberStlVector< std::wstring >(L"filePaths", m_filePaths);
}

}
