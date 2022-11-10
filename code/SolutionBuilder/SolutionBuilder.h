/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Misc/CommandLine.h"

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

class Solution;

/*! Solution builder generator base class.
 *
 * Each solution generate is derived from this
 * base class.
 */
class T_DLLCLASS SolutionBuilder : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool create(const CommandLine& cmdLine) = 0;

	virtual bool generate(Solution* solution) = 0;

	virtual void showOptions() const = 0;
};

	}
}

