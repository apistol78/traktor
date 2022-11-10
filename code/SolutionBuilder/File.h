/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <set>
#include <string>
#include "Core/Io/Path.h"
#include "SolutionBuilder/ProjectItem.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOLUTIONBUILDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sb
	{

class T_DLLCLASS File : public ProjectItem
{
	T_RTTI_CLASS;

public:
	void setFileName(const std::wstring& fileName);

	const std::wstring& getFileName() const;

	void setExcludeFilter(const std::wstring& excludeFilter);

	const std::wstring& getExcludeFilter() const;

	void getSystemFiles(const Path& sourcePath, std::set< Path >& outFiles) const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_fileName;
	std::wstring m_excludeFilter;
};

	}
}

