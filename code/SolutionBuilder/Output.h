/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vector>
#include "Core/Io/StringOutputStream.h"

namespace traktor::sb
{

class Output : public Object
{
	T_RTTI_CLASS;

public:
	explicit Output(const std::vector< std::wstring >& sections);

	void print(const std::wstring& str);

	void printLn(const std::wstring& str);

	void printSection(int32_t id);

	std::wstring getProduct() const;

private:
	const std::vector< std::wstring >& m_sections;
	StringOutputStream m_ss;
};

}
