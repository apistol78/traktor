/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_online_LobbyFilter_H
#define traktor_online_LobbyFilter_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ONLINE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif 

namespace traktor
{
	namespace online
	{

class T_DLLCLASS LobbyFilter : public Object
{
	T_RTTI_CLASS;

public:
	enum ComparisonType
	{
		CtEqual,
		CtNotEqual,
		CtLess,
		CtLessEqual,
		CtGreater,
		CtGreaterEqual
	};

	enum DistanceType
	{
		DtUnspecified,
		DtLocal,
		DtNear,
		DtFar,
		DtInfinity
	};

	struct StringComparison
	{
		std::wstring key;
		std::wstring value;
		ComparisonType comparison;
	};

	struct NumberComparison
	{
		std::wstring key;
		int32_t value;
		ComparisonType comparison;
	};

	LobbyFilter();

	void addComparison(const std::wstring& key, const std::wstring& value, ComparisonType comparison);

	void addComparison(const std::wstring& key, int32_t value, ComparisonType comparison);

	void setDistance(DistanceType distance);

	void setSlots(int32_t slots);

	void setCount(int32_t count);

	const std::vector< StringComparison >& getStringComparisons() const { return m_stringComparisons; }

	const std::vector< NumberComparison >& getNumberComparisons() const { return m_numberComparisons; }

	DistanceType getDistance() const { return m_distance; }

	int32_t getSlots() const { return m_slots; }

	int32_t getCount() const { return m_count; }

private:
	std::vector< StringComparison > m_stringComparisons;
	std::vector< NumberComparison > m_numberComparisons;
	DistanceType m_distance;
	int32_t m_slots;
	int32_t m_count;
};

	}
}

#endif	// traktor_online_LobbyFilter_H
