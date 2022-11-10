/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXBuildTool.h"

namespace traktor
{
	namespace sb
	{

class SolutionBuilderMsvcVCXCustomBuildTool : public SolutionBuilderMsvcVCXBuildTool
{
	T_RTTI_CLASS;

public:
	virtual bool generateProject(
		GeneratorContext& context,
		Solution* solution,
		Project* project,
		const std::wstring& filter,
		const Path& fileName,
		OutputStream& os
	) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_command;
	std::wstring m_message;
	std::wstring m_outputs;
};

	}
}

