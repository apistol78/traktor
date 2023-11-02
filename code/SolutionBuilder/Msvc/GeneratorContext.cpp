/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <sstream>
#include "Core/Io/FileSystem.h"
#include "Core/Misc/MD5.h"
#include "SolutionBuilder/Msvc/GeneratorContext.h"

namespace traktor::sb
{

GeneratorContext::GeneratorContext(bool includeExternal)
:	m_includeExternal(includeExternal)
{
}

void GeneratorContext::set(const std::wstring& key, const std::wstring& value)
{
	m_values[key] = value;
}

std::wstring GeneratorContext::get(const std::wstring& key) const
{
	auto it = m_values.find(key);
	return it != m_values.end() ? it->second : L"";
}

std::wstring GeneratorContext::format(const std::wstring& option) const
{
	std::wstring final;

	size_t s = 0, n = option.find(L"%(");
	while (n != std::wstring::npos)
	{
		final += option.substr(s, n - s);

		const size_t e = option.find(L")", n + 2);
		if (e == std::wstring::npos)
			break;

		const std::wstring key = option.substr(n + 2, e - n - 2);
		const auto it = m_values.find(key);
		if (it != m_values.end())
			final += it->second;
		else
			final += L"%(" + key + L")";

		s = e + 1;
		n = option.find(L"%(", s);
	}

	final += option.substr(s);

	return final;
}

std::wstring GeneratorContext::getProjectRelativePath(const std::wstring& path, bool resolve) const
{
	Path resolvedPath(path);

	// Don't modify absolute paths.
	if (!resolvedPath.isRelative())
		return resolve ? resolvedPath.getPathName() : path;

	// Transform path into relative to vcproj file.
	Path relativePath;
	FileSystem& fileSystem = FileSystem::getInstance();
	if (fileSystem.getRelativePath(
		fileSystem.getAbsolutePath(path),
		fileSystem.getAbsolutePath(get(L"PROJECT_PATH")),
		relativePath
	))
		return relativePath.getPathName();

	return resolve ? resolvedPath.getPathName() : path;
}

std::wstring GeneratorContext::generateGUID(const std::wstring& key) const
{
	MD5 md5;

	md5.begin();
	md5.feed(key.c_str(), int(key.length() * sizeof(wchar_t)));
	md5.end();

	const uint8_t* cs = reinterpret_cast< const uint8_t* >(md5.get());

	std::wstringstream ss;
	ss << L"{";
	for (int i = 0; i < 16; ++i)
	{
		wchar_t hex[] =
		{
			L"0123456789ABCDEF"[(cs[i] >> 4) & 0xf],
			L"0123456789ABCDEF"[cs[i] & 0xf]
		};
		ss << hex[0] << hex[1];
		if (i == 3 || i == 5 || i == 7 || i == 9)
			ss << L"-";
	}
	ss << L"}";

	return ss.str();
}

}
