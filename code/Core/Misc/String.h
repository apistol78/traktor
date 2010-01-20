#ifndef traktor_String_H
#define traktor_String_H

#include <algorithm>
#include <cctype>
#include <limits>
#include <sstream>
#include <string>
#include "Core/Config.h"

#if defined(max)
#	undef max
#endif

namespace traktor
{

/*! \brief Replace all occurrences of a character into another character.
 * \ingroup Core
 *
 * \param str Source string.
 * \param ch Character to search for.
 * \param rep Character to replace with.
 * \return Transformed string.
 */
template < typename String >
String replaceAll(const String& str, typename String::value_type ch, typename String::value_type rep)
{
	String cpy = str;
	std::replace(cpy.begin(), cpy.end(), ch, rep);
	return cpy;
}

/*! \brief Replace all occurrences of a substring into another string.
 * \ingroup Core
 *
 * \param str Source string.
 * \param ndl Needle.
 * \param rep Replacing string.
 * \return Transformed string.
 */
template < typename String >
String replaceAll(const String& str, const String& ndl, const String& rep)
{
	String cpy = str; size_t p;
	while ((p = cpy.find(ndl)) != -1)
		cpy = cpy.substr(0, p) + rep + cpy.substr(p + ndl.length());
	return cpy;
}

/*! \brief Apply transformation of an entire string.
 * \ingroup Core
 *
 * \param str Source string.
 * \param func Pointer to transformation function.
 * \return Transformed string.
 */
template < typename String >
String transform(const String& str, int (*func)(int c))
{
	String cpy = str;
	std::transform(cpy.begin(), cpy.end(), cpy.begin(), func);
	return cpy;
}

/*! \brief Transform a string to all uppercase.
 * \ingroup Core
 *
 * \param str Source string.
 * \return All uppercase string.
 */
inline std::wstring toUpper(const std::wstring& str)
{
	return transform< std::wstring >(str, std::toupper);
}

/*! \brief Transform a string to all lowercase.
 * \ingroup Core
 *
 * \param str Source string.
 * \return All lowercase string.
 */
inline std::wstring toLower(const std::wstring& str)
{
	return transform< std::wstring >(str, std::tolower);
}

/*! \brief See if the start of a string matches a given string.
 * \ingroup Core
 *
 * \param str Source string.
 * \param start Match string.
 * \return True if source string starts with given match string.
 */
inline bool startsWith(const std::wstring& str, const std::wstring& start)
{
	if (str.length() < start.length())
		return false;
	if (start.length() <= 0)
		return true;
	return bool(str.substr(0, start.length()) == start);
}

/*! \brief See if the end of a string matches a given string.
 * \ingroup Core
 *
 * \param str Source string.
 * \param end Match string.
 * \return True if source string ends with given match string.
 */
inline bool endsWith(const std::wstring& str, const std::wstring& end)
{
	if (str.length() < end.length())
		return false;
	if (end.length() <= 0)
		return true;
	return bool(str.substr(str.length() - end.length(), end.length()) == end);
}

/*! \brief Perform a lexicographically compare of strings.
 * \ingroup Core
 *
 * \param a Left hand string.
 * \param b Right hand string.
 * \return See basic_string::compare().
 */
inline int compareIgnoreCase(const std::wstring& a, const std::wstring& b)
{
	return toLower(a).compare(toLower(b));
}

/*! \brief Trim leading white-space.
 * \ingroup Core
 */
inline std::wstring ltrim(const std::wstring& a, const std::wstring& ws = L" \t\n\r")
{
	size_t i = a.find_first_not_of(ws);
	if (i == std::wstring::npos)
		return L"";
	return a.substr(i);
}

/*! \brief Trim trailing white-space.
 * \ingroup Core
 */
inline std::wstring rtrim(const std::wstring& a, const std::wstring& ws = L" \t\n\r")
{
	size_t i = a.find_last_not_of(ws);
	if (i == std::wstring::npos)
		return L"";
	return a.substr(0, i + 1);
}

/*! \brief Trim white-space.
 * \ingroup Core
 */
inline std::wstring trim(const std::wstring& a)
{
	return ltrim(rtrim(a));
}

/*! \brief Convert value to literal.
 * \ingroup Core
 */
template < typename ValueType >
inline std::wstring toString(ValueType value)
{
	std::wstringstream ss; ss << value;
	return ss.str();
}

/*! \brief Convert literal to value.
 * \ingroup Core
 */
template < typename ValueType >
inline ValueType parseString(const std::string& text)
{
	ValueType value = std::numeric_limits< ValueType >::has_signaling_NaN ?  std::numeric_limits< ValueType >::signaling_NaN() : std::numeric_limits< ValueType >::max();
	std::stringstream ss(text); ss >> value;
	return value;
}

/*! \brief Convert literal to value.
 * \ingroup Core
 */
template < typename ValueType >
inline ValueType parseString(const std::wstring& text)
{
	ValueType value = std::numeric_limits< ValueType >::has_signaling_NaN ?  std::numeric_limits< ValueType >::signaling_NaN() : 0;
	std::wstringstream ss(text); ss >> value;
	return value;
}

/*! \brief Convert literal to boolean.
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
		int32_t number = parseString< int32_t >(text);
		if (number > 0)
			return true;
	}
	return false;
}


}

#endif	// traktor_String_H
