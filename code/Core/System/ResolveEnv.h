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
#include "Core/System/OS.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Environment;
class PropertyGroup;

/*! Resolve all environment variable references in input string. */
std::wstring T_DLLCLASS resolveEnv(const std::wstring& s, const Environment* env);

/*! Set variables of group, which doesn't already exist, into process environment; empty values are ignored. */
void T_DLLCLASS applyEnvironment(const PropertyGroup* variables);

/*! Split command line into argv. */
bool T_DLLCLASS splitCommandLine(const std::wstring& commandLine, AlignedVector< std::wstring >& outArgv);

}
