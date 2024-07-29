/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::online
{

class T_DLLCLASS LobbyFilter : public Object
{
	T_RTTI_CLASS;

public:
	enum class Comparison
	{
		Equal,
		NotEqual,
		Less,
		LessEqual,
		Greater,
		GreaterEqual
	};

	enum class Distance
	{
		Unspecified,
		Local,
		Near,
		Far,
		Infinity
	};

	struct StringComparison
	{
		std::wstring key;
		std::wstring value;
		Comparison comparison;
	};

	struct NumberComparison
	{
		std::wstring key;
		int32_t value;
		Comparison comparison;
	};

	void addComparison(const std::wstring& key, const std::wstring& value, Comparison comparison);

	void addComparison(const std::wstring& key, int32_t value, Comparison comparison);

	void setDistance(Distance distance);

	void setSlots(int32_t slots);

	void setCount(int32_t count);

	const AlignedVector< StringComparison >& getStringComparisons() const { return m_stringComparisons; }

	const AlignedVector< NumberComparison >& getNumberComparisons() const { return m_numberComparisons; }

	Distance getDistance() const { return m_distance; }

	int32_t getSlots() const { return m_slots; }

	int32_t getCount() const { return m_count; }

private:
	AlignedVector< StringComparison > m_stringComparisons;
	AlignedVector< NumberComparison > m_numberComparisons;
	Distance m_distance = Distance::Infinity;
	int32_t m_slots = 0;
	int32_t m_count = -1;
};

}
