#ifndef traktor_String_H
#define traktor_String_H

#include <algorithm>
#include <cctype>
#include <limits>
#include <sstream>
#include <string>
#include "Core/Config.h"
#include "Core/Io/StringOutputStream.h"

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
	String cpy = str; size_t p = 0;
	while ((p = cpy.find(ndl, p)) != -1)
	{
		cpy = cpy.substr(0, p) + rep + cpy.substr(p + ndl.length());
		p += rep.length();
	}
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

/*! \brief Implode set of strings into a single string.
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

/*! \brief Transform a string to all uppercase.
 * \ingroup Core
 *
 * \param str Source string.
 * \return All uppercase string.
 */
template < typename String >
String toUpper(const String& str)
{
	return transform< String >(str, std::toupper);
}

/*! \brief Transform a string to all lowercase.
 * \ingroup Core
 *
 * \param str Source string.
 * \return All lowercase string.
 */
template < typename String >
String toLower(const String& str)
{
	return transform< String >(str, std::tolower);
}

/*! \brief See if the start of a string matches a given string.
 * \ingroup Core
 *
 * \param str Source string.
 * \param start Match string.
 * \return True if source string starts with given match string.
 */
template < typename StringType >
bool startsWith(const StringType& str, const StringType& start)
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
template < typename StringType >
bool endsWith(const StringType& str, const StringType& end)
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
template < typename StringType >
int compareIgnoreCase(const StringType& a, const StringType& b)
{
	return toLower< StringType >(a).compare(toLower< StringType >(b));
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
std::wstring toString(const ValueType& value)
{
	StringOutputStream ss; ss << value;
	return ss.str();
}

/*! \brief Convert value to literal.
 * \ingroup Core
 */
inline std::wstring toString(float value, int32_t decimals = 6)
{
	StringOutputStream ss;
	ss.setDecimals(decimals);
	ss << value;
	return ss.str();
}

/*! \brief Convert value to literal.
 * \ingroup Core
 */
inline std::wstring toString(double value, int32_t decimals = 6)
{
	StringOutputStream ss;
	ss.setDecimals(decimals);
	ss << value;
	return ss.str();
}

/*! \brief Convert literal to value.
 * \ingroup Core
 */
template < typename ValueType >
ValueType parseString(const std::string& text, const ValueType& defaultValue)
{
	ValueType value = defaultValue;
	if (startsWith< std::string >(text, "0x"))
		std::stringstream(text.substr(2)) >> std::hex >> value;
	else
		std::stringstream(text) >> value;
	return value;
}

/*! \brief Convert literal to value.
 * \ingroup Core
 */
template < typename ValueType >
ValueType parseString(const std::string& text)
{
	return parseString(text, std::numeric_limits< ValueType >::has_signaling_NaN ?  std::numeric_limits< ValueType >::signaling_NaN() : 0);
}

/*! \brief Convert literal to value.
 * \ingroup Core
 */
template < typename ValueType >
ValueType parseString(const std::wstring& text, const ValueType& defaultValue)
{
	ValueType value = defaultValue;
	if (startsWith< std::wstring >(text, L"0x"))
		std::wstringstream(text.substr(2)) >> std::hex >> value;
	else
		std::wstringstream(text) >> value;
	return value;
}

/*! \brief Convert literal to boolean.
 * \ingroup Core
 */
template < >
inline float parseString< float >(const std::wstring& text, const float& defaultValue)
{
	float value = defaultValue;
	std::wstringstream(text) >> value;
	return (value != -value) ? value : 0.0f;
}


/*! \brief Convert literal to value.
 * \ingroup Core
 */
template < typename ValueType >
ValueType parseString(const std::wstring& text)
{
	return parseString(text, std::numeric_limits< ValueType >::has_signaling_NaN ?  std::numeric_limits< ValueType >::signaling_NaN() : 0);
}

/*! \brief Convert literal to boolean.
 * \ingroup Core
 */
template < >
inline bool parseString< bool >(const std::wstring& text)
{
	if (compareIgnoreCase< std::wstring >(text, L"true") == 0)
		return true;
	else if (compareIgnoreCase< std::wstring >(text, L"yes") == 0)
		return true;
	else
	{
		int32_t number = parseString< int32_t >(text);
		if (number > 0)
			return true;
	}
	return false;
}

/*! \brief Two-way split.
 * \ingroup Core
 */
inline bool split(const std::wstring& str, wchar_t delim, std::wstring& outLeft, std::wstring& outRight)
{
	size_t p = str.find_first_of(delim);
	if (p == str.npos)
		return false;

	outLeft = str.substr(0, p);
	outRight = str.substr(p + 1);

	return true;
}

}

#endif	// traktor_String_H
