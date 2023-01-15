/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/File.h"
#include "Core/Io/FileSystem.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Core/Misc/WildCompare.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "SolutionBuilder/File.h"

namespace traktor::sb
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"File", 1, File, ProjectItem)

void File::setFileName(const std::wstring& fileName)
{
	m_fileName = fileName;
}

const std::wstring& File::getFileName() const
{
	return m_fileName;
}

void File::setExcludeFilter(const std::wstring& excludeFilter)
{
	m_excludeFilter = excludeFilter;
}

const std::wstring& File::getExcludeFilter() const
{
	return m_excludeFilter;
}

void File::getSystemFiles(const Path& sourcePath, std::set< Path >& outFiles) const
{
	const Path path = (FileSystem::getInstance().getAbsolutePath(sourcePath) + Path(m_fileName)).normalized();

	// Parse exclude filters.
	AlignedVector< WildCompare > wcs;
	if (!m_excludeFilter.empty())
	{
		StringSplit< std::wstring > ss(m_excludeFilter, L";");
		for (auto s : ss)
			wcs.push_back(WildCompare(s));
	}

	RefArray< traktor::File > files;
	FileSystem::getInstance().find(path, files);
	for (auto file : files)
	{
		const std::wstring fn = file->getPath().getFileName();
		if (fn == L"." || fn == L"..")
			continue;

		if (
			!file->isDirectory() &&
			!file->isHidden()
		)
		{
			bool include = true;
			for (const auto& wc : wcs)
			{
				include &= (wc.match(fn, WildCompare::CaseSensitive) == 0);
				if (!include)
					break;
			}
			if (include)
				outFiles.insert(file->getPath().getPathName());
		}
	}
}

void File::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"fileName", m_fileName);
	if (s.getVersion< File >() >= 1)
		s >> Member< std::wstring >(L"excludeFilter", m_excludeFilter);
	ProjectItem::serialize(s);
}

}
