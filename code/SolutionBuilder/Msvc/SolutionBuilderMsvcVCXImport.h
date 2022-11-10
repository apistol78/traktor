/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXImportCommon.h"

namespace traktor
{
	namespace sb
	{

class SolutionBuilderMsvcVCXImport : public SolutionBuilderMsvcVCXImportCommon
{
	T_RTTI_CLASS;

public:
	SolutionBuilderMsvcVCXImport() = default;

	explicit SolutionBuilderMsvcVCXImport(
		const std::wstring& label,
		const std::wstring& project,
		const std::wstring& condition
	);

	virtual bool generate(OutputStream& os) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_label;
	std::wstring m_project;
	std::wstring m_condition;
};

	}
}

