/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

private:
	struct Includes
	{
		SmallSet< std::wstring > files;
	};

	SmallSet< std::wstring > m_includePaths;
	SmallMap< std::wstring, Includes* > m_cache;

	/*! Scan header dependencies of source file. */
	const Includes* scan(const std::wstring& fileName);
};

}