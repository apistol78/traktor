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
#include "Core/RefArray.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor::sb
{

class GeneratorContext;
class Solution;
class Project;
class ProjectItem;
class Configuration;

class SolutionBuilderMsvcVCXDefinition : public ISerializable
{
	T_RTTI_CLASS;

public:
	bool generate(
		GeneratorContext& context,
		const Solution* solution,
		const Project* project,
		const Configuration* configuration,
		OutputStream& os
	) const;

	virtual void serialize(ISerializer& s) override final;

private:
	struct Option
	{
		std::wstring name;
		std::wstring value;

		void serialize(ISerializer& s);
	};

	std::wstring m_name;
	std::wstring m_fileTypes;
	bool m_resolvePaths = true;
	std::vector< Option > m_options;

	void collectAdditionalLibraries(
		const Solution* solution,
		const Project* project,
		const Configuration* configuration,
		std::set< std::wstring >& outAdditionalLibraries,
		std::set< std::wstring >& outAdditionalLibraryPaths
	) const;

	void findDefinitions(
		GeneratorContext& context,
		const Solution* solution,
		const Project* project,
		const RefArray< ProjectItem >& items
	) const;
};

}
