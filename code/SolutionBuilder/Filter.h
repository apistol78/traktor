/*
 * TRAKTOR
 * Copyright (c) 2022-2025 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "SolutionBuilder/ProjectItem.h"

#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOLUTIONBUILDER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sb
{

class T_DLLCLASS Filter : public ProjectItem
{
	T_RTTI_CLASS;

public:
	void setName(const std::wstring& name);

	const std::wstring& getName() const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_name;
};

}
