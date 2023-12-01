/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <limits>
#include <sstream>
#include <string>
#include "Core/Config.h"
#include "Core/Io/StringOutputStream.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Replace all occurrences of a character into another character.
 * \ingroup Core
 *
 * \param str Source string.
 * \param ch Character to search for.
 * \param rep Character to replace with.
 * \return Transformed string.
 */
std::string T_DLLCLASS replaceAll(const std::string& str, typename std::string::value_type ch, typename std::string::value_type rep);

/*! Replace all occurrences of a character into another character.
 * \ingroup Core
 *
 * \param str Source string.
 * \param ch Character to search for.
 * \param rep Character to replace with.
 * \return Transformed string.
 */
std::wstring T_DLLCLASS replaceAll(const std::wstring& str, typename std::wstring::value_type ch, typename std::wstring::value_type rep);

/*! Replace all occurrences of a substring into another string.
 * \ingroup Core
 *
 * \param str Source string.
 * \param ndl Needle.
 * \param rep Replacing string.
 * \return Transformed string.
 */
std::string T_DLLCLASS replaceAll(const std::string& str, const std::string& ndl, const std::string& rep);

/*! Replace all occurrences of a substring into another string.
 * \ingroup Core
 *
 * \param str Source string.
 * \param ndl Needle.
 * \param rep Replacing string.
 * \return Transformed string.
 */
std::wstring T_DLLCLASS replaceAll(const std::wstring& str, const std::wstring& ndl, const std::wstring& rep);

/*! Apply transformation of an entire string.
 * \ingroup Core
 *
 * \param str Source string.
 * \param func Pointer to transformation function.
 * \return Transformed string.
 */
std::string T_DLLCLASS transform(const std::string_view& str, int (*func)(int c));

/*! Apply transformation of an entire string.
 * \ingroup Core
 *
 * \param str Source string.
 * \param func Pointer to transformation function.
 * \return Transformed string.
 */
std::wstring T_DLLCLASS transform(const std::wstring_view& str, int (*func)(int c));

/*! Implode set of strings into a single string.
 * \ingroup Core
 *
 * \param begin From iterator.
 * \param end To iterator.
 * \param delim Delimiter string.
 * \return Merged string.
 */
template < typename Iterator >
typename Iterator::value_type implode(const Iterator& begin, const Iterator& end, const typename Iterator::value_type& delim)
{
	typename Iterator::value_type tmp;
	Iterator i = begin;
	if (i != end)
	{
		tmp = *i++;
		for (; i != end; ++i)
			tmp += delim + *i;
	}
	return tmp;
}

/*! Transform a string to all uppercase.
 * \ingroup Core
 *
 * \param str Source string.
 * \return All uppercase string.
 */
std::string T_DLLCLASS toUpper(const std::string_view& str);

/*! Transform a string to all uppercase.
 * \ingroup Core
 *
 * \param str Source string.
 * \return All uppercase string.
 */
std::wstring T_DLLCLASS toUpper(const std::wstring_view& str);

/*! Transform a string to all lowercase.
 * \ingroup Core
 *
 * \param str Source string.
 * \return All lowercase string.
 */
std::string T_DLLCLASS toLower(const std::string_view& str);

/*! Transform a string to all lowercase.
 * \ingroup Core
 *
 * \param str Source string.
 * \return All lowercase string.
 */
std::wstring T_DLLCLASS toLower(const std::wstring_view& str);

/*! See if the start of a string matches a given string.
 * \ingroup Core
 *
 * \param str Source string.
 * \param start Match string.
 * \return True if source string starts with given match string.
 */
bool T_DLLCLASS startsWith(const std::string_view& str, const std::string_view& start);

/*! See if the start of a string matches a given string.
 * \ingroup Core
 *
 * \param str Source string.
 * \param start Match string.
 * \return True if source string starts with given match string.
 */
bool T_DLLCLASS startsWith(const std::wstring_view& str, const std::wstring_view& start);

/*! See if the end of a string matches a given string.
 * \ingroup Core
 *
 * \param str Source string.
 * \param end Match string.
 * \return True if source string ends with given match string.
 */
bool T_DLLCLASS endsWith(const std::string_view& str, const std::string_view& end);

/*! See if the end of a string matches a given string.
 * \ingroup Core
 *
 * \param str Source string.
 * \param end Match string.
 * \return True if source string ends with given match string.
 */
bool T_DLLCLASS endsWith(const std::wstring_view& str, const std::wstring_view& end);

/*! Perform a lexicographically compare of strings.
 * \ingroup Core
 *
 * \param a Left hand string.
 * \param b Right hand string.
 * \return See basic_string::compare().
 */
int T_DLLCLASS compareIgnoreCase(const std::string_view& a, const std::string_view& b);

/*! Perform a lexicographically compare of strings.
 * \ingroup Core
 *
 * \param a Left hand string.
 * \param b Right hand string.
 * \return See basic_string::compare().
 */
int T_DLLCLASS compareIgnoreCase(const std::wstring_view& a, const std::wstring_view& b);

/*! Trim leading white-space.
 * \ingroup Core
 */
std::wstring T_DLLCLASS ltrim(const std::wstring& a, const std::wstring& ws = L" \t\n\r");

/*! Trim trailing white-space.
 * \ingroup Core
 */
std::wstring T_DLLCLASS rtrim(const std::wstring& a, const std::wstring& ws = L" \t\n\r");

/*! Trim white-space.
 * \ingroup Core
 */
std::wstring T_DLLCLASS trim(const std::wstring& a);

/*! Convert value to literal.
 * \ingroup Core
 */
template < typename ValueType >
std::wstring toString(const ValueType& value)
{
	StringOutputStream ss; ss << value;
	return ss.str();
}

/*! Convert value to literal.
 * \ingroup Core
 */
std::wstring T_DLLCLASS toString(float value, int32_t decimals = 6);

/*! Convert value to literal.
 * \ingroup Core
 */
std::wstring T_DLLCLASS toString(double value, int32_t decimals = 6);

/*! Convert literal to value.
 * \ingroup Core
 */
template < typename ValueType >
ValueType parseString(const std::string& text, const ValueType& defaultValue)
{
	ValueType value = defaultValue;
	if (startsWith(text, "0x"))
		std::stringstream(text.substr(2)) >> std::hex >> value;
	else
		std::stringstream(text) >> value;
	return value;
}

/*! Convert literal to value.
 * \ingroup Core
 */
template < typename ValueType >
ValueType parseString(const std::string& text)
{
	return parseString(text, std::numeric_limits< ValueType >::has_signaling_NaN ?  std::numeric_limits< ValueType >::signaling_NaN() : 0);
}

/*! Convert literal to value.
 * \ingroup Core
 */
template < typename ValueType >
ValueType parseString(const std::wstring& text, const ValueType& defaultValue)
{
	ValueType value = defaultValue;
	if (startsWith(text, L"0x"))
		std::wstringstream(text.substr(2)) >> std::hex >> value;
	else
		std::wstringstream(text) >> value;
	return value;
}

/*! Convert literal to boolean.
 * \ingroup Core
 */
template < >
inline float parseString< float >(const std::wstring& text, const float& defaultValue)
{
	float value = defaultValue;
	std::wstringstream(text) >> value;
	return (value != -value) ? value : 0.0f;
}


/*! Convert literal to value.
 * \ingroup Core
 */
template < typename ValueType >
ValueType parseString(const std::wstring& text)
{
	return parseString(text, std::numeric_limits< ValueType >::has_signaling_NaN ?  std::numeric_limits< ValueType >::signaling_NaN() : 0);
}

/*! Convert literal to boolean.
 * \ingroup Core
 */
template < >
inline bool parseString< bool >(const std::wstring& text)
{
	if (compareIgnoreCase(text, L"true") == 0)
		return true;
	else if (compareIgnoreCase(text, L"yes") == 0)
		return true;
	else
	{
		const int32_t number = parseString< int32_t >(text);
		if (number > 0)
			return true;
	}
	return false;
}

/*! Two-way split.
 * \ingroup Core
 */
bool T_DLLCLASS split(const std::wstring& str, wchar_t delim, std::wstring& outLeft, std::wstring& outRight);

/*! String formatting helper.
 * \ingroup Core
 */
std::wstring T_DLLCLASS str(const wchar_t* const format, ...);

/*! Byte size formatting helper.
 * \ingroup Core
 */
std::wstring T_DLLCLASS formatByteSize(uint64_t size);

/*! Duration formatting helper.
 * \ingroup Core
 */
std::wstring T_DLLCLASS formatDuration(double duration);

}
