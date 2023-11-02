/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "SolutionBuilder/Msvc/SolutionBuilderMsvcVCXImportCommon.h"

namespace traktor::sb
{

class SolutionBuilderMsvcVCXImportGroup : public SolutionBuilderMsvcVCXImportCommon
{
	T_RTTI_CLASS;

public:
	SolutionBuilderMsvcVCXImportGroup() = default;

	explicit SolutionBuilderMsvcVCXImportGroup(const std::wstring& label, const std::wstring& condition);

	void addImport(SolutionBuilderMsvcVCXImportCommon* import);

	virtual bool generate(OutputStream& os) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	std::wstring m_label;
	std::wstring m_condition;
	RefArray< SolutionBuilderMsvcVCXImportCommon > m_imports;
};

}
