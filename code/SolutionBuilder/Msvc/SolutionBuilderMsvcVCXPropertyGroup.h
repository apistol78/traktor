/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <map>
#include "Core/Io/OutputStream.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor
{
	namespace sb
	{

class GeneratorContext;
class Solution;
class Project;
class ProjectItem;

class SolutionBuilderMsvcVCXPropertyGroup : public ISerializable
{
	T_RTTI_CLASS;

public:
	bool generate(
		GeneratorContext& context,
		const Solution* solution,
		const Project* project,
		OutputStream& os
	) const;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_label;
	std::wstring m_condition;
	std::map< std::wstring, std::wstring > m_values;
};

	}
}

