/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include <string>
#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Containers/SmallSet.h"

namespace traktor::sb
{

class HeaderScanner : public Object
{
	T_RTTI_CLASS;

public:
	virtual ~HeaderScanner();

	void removeAllIncludePaths();

	void addIncludePath(const std::wstring& includePath);

	bool get(const std::wstring& fileName, const std::wstring& projectPath, SmallSet< std::wstring >& outHeaderFiles);

	/*! Get absolute paths of files directly included by file. */
	const SmallSet< std::wstring >& getIncludes(const std::wstring& fileName);

private:
	struct Includes
	{
		SmallSet< std::wstring > files;
	};

	SmallSet< std::wstring > m_includePaths;
	SmallMap< std::wstring, Includes* > m_cache;
	std::map< std::wstring, std::wstring > m_resolved; //!< Too many entries for SmallMap's sorted inserts.

	/*! Scan header dependencies of source file. */
	const Includes* scan(const std::wstring& fileName);

	/*! Get normalized name of file, empty if file doesn't exist. */
	std::wstring resolve(const std::wstring& fileName);
};

}