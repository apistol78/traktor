/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Io/StringOutputStream.h"
#include "Run/App/IOutput.h"

namespace traktor
{
	namespace run
	{

/*! Output writer with interleaved sections.
 * \ingroup Run
 */
class ProduceOutput : public IOutput
{
	T_RTTI_CLASS;

public:
	virtual void print(const std::wstring& s) override final;

	virtual void printLn(const std::wstring& s) override final;

	void printSection(int32_t id);

	int32_t addSection(const std::wstring& section);

	std::wstring getProduct() const;

private:
	std::vector< std::wstring > m_sections;
	StringOutputStream m_ss;
};

	}
}

