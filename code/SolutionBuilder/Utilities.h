/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Containers/SmallMap.h"
#include "Core/Containers/SmallSet.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOLUTIONBUILDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Path;

}

namespace traktor::sb
{

class Project;

bool writeFileIfMismatch(const std::wstring& fileName, const AlignedVector< uint8_t >& data);

/*! Collect include and library paths, per configuration, of project and inherited from its dependencies. */
void T_DLLCLASS flattenIncludePaths(
	const Path& solutionPath,
	const Project* project,
	SmallSet< const Project* >& visited,
	SmallMap< std::wstring, SmallSet< std::wstring > >& outConfigurationIncludePaths,
	SmallMap< std::wstring, SmallSet< std::wstring > >& outConfigurationLibraryPaths);

}
