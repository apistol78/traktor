/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include <vector>
#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Compare string with wild-card matching.
 * \ingroup Core
 */
class T_DLLCLASS WildCompare
{
public:
	enum CompareMode
	{
		IgnoreCase,
		CaseSensitive
	};

	WildCompare(const std::wstring& mask);

	/*! Match string with wild-card pattern. */
	bool match(const std::wstring& str, CompareMode mode = IgnoreCase, std::vector< std::wstring >* outPieces = 0) const;

	/*! Insert string pieces at wild-card tokens */
	std::wstring merge(const std::vector< std::wstring >& pieces) const;

private:
	std::vector< std::wstring > m_nowild;
	bool m_beginWild;
	bool m_endWild;
};

}

