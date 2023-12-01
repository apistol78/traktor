/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <locale>
#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>

#include "Core/Misc/String.h"

namespace traktor
{

std::string replaceAll(const std::string& str, typename std::string::value_type ch, typename std::string::value_type rep)
{
	std::string cpy = str;
	std::replace(cpy.begin(), cpy.end(), ch, rep);
	return cpy;
}

std::wstring replaceAll(const std::wstring& str, typename std::wstring::value_type ch, typename std::wstring::value_type rep)
{
	std::wstring cpy = str;
	std::replace(cpy.begin(), cpy.end(), ch, rep);
	return cpy;
}

std::string replaceAll(const std::string& str, const std::string& ndl, const std::string& rep)
{
	std::string cpy = str; size_t p = 0;
	while ((p = cpy.find(ndl, p)) != cpy.npos)
	{
		cpy = cpy.substr(0, p) + rep + cpy.substr(p + ndl.length());
		p += rep.length();
	}
	return cpy;
}

std::wstring replaceAll(const std::wstring& str, const std::wstring& ndl, const std::wstring& rep)
{
	std::wstring cpy = str; size_t p = 0;
	while ((p = cpy.find(ndl, p)) != cpy.npos)
	{
		cpy = cpy.substr(0, p) + rep + cpy.substr(p + ndl.length());
		p += rep.length();
	}
	return cpy;
}

std::string transform(const std::string_view& str, int (*func)(int c))
{
	std::string cpy(str);
	std::transform(cpy.begin(), cpy.end(), cpy.begin(), func);
	return cpy;
}

std::wstring transform(const std::wstring_view& str, int (*func)(int c))
{
	std::wstring cpy(str);
	std::transform(cpy.begin(), cpy.end(), cpy.begin(), func);
	return cpy;
}

std::string toUpper(const std::string_view& str)
{
	return transform(str, toupper);
}

std::wstring toUpper(const std::wstring_view& str)
{
	return transform(str, toupper);
}

std::string toLower(const std::string_view& str)
{
	return transform(str, tolower);
}

std::wstring toLower(const std::wstring_view& str)
{
	return transform(str, tolower);
}

bool startsWith(const std::string_view& str, const std::string_view& start)
{
	if (str.length() < start.length())
		return false;
	if (start.length() <= 0)
		return true;
	return (bool)(str.substr(0, start.length()) == start);
}

bool startsWith(const std::wstring_view& str, const std::wstring_view& start)
{
	if (str.length() < start.length())
		return false;
	if (start.length() <= 0)
		return true;
	return (bool)(str.substr(0, start.length()) == start);
}

bool endsWith(const std::string_view& str, const std::string_view& end)
{
	if (str.length() < end.length())
		return false;
	if (end.length() <= 0)
		return true;
	return (bool)(str.substr(str.length() - end.length(), end.length()) == end);
}

bool endsWith(const std::wstring_view& str, const std::wstring_view& end)
{
	if (str.length() < end.length())
		return false;
	if (end.length() <= 0)
		return true;
	return (bool)(str.substr(str.length() - end.length(), end.length()) == end);
}

int compareIgnoreCase(const std::string_view& a, const std::string_view& b)
{
	return toLower(a).compare(toLower(b));
}

int compareIgnoreCase(const std::wstring_view& a, const std::wstring_view& b)
{
	return toLower(a).compare(toLower(b));
}

std::wstring ltrim(const std::wstring& a, const std::wstring& ws)
{
	const size_t i = a.find_first_not_of(ws);
	if (i == std::wstring::npos)
		return L"";
	return a.substr(i);
}

std::wstring rtrim(const std::wstring& a, const std::wstring& ws)
{
	const size_t i = a.find_last_not_of(ws);
	if (i == std::wstring::npos)
		return L"";
	return a.substr(0, i + 1);
}

std::wstring trim(const std::wstring& a)
{
	return ltrim(rtrim(a));
}

std::wstring toString(float value, int32_t decimals)
{
	StringOutputStream ss;
	ss.setDecimals(decimals);
	ss << value;
	return ss.str();
}

std::wstring toString(double value, int32_t decimals)
{
	StringOutputStream ss;
	ss.setDecimals(decimals);
	ss << value;
	return ss.str();
}

bool split(const std::wstring& str, wchar_t delim, std::wstring& outLeft, std::wstring& outRight)
{
	const size_t p = str.find_first_of(delim);
	if (p == str.npos)
		return false;

	outLeft = str.substr(0, p);
	outRight = str.substr(p + 1);
	return true;
}

std::wstring str(const wchar_t* const format, ...)
{
	wchar_t buf[1024];
	va_list a;

	va_start(a, format);
	vswprintf(buf, sizeof_array(buf), format, a);
	va_end(a);

	return buf;
}

std::wstring formatByteSize(uint64_t size)
{
	if (size >= 1024 * 1024 * 1024)
		return str(L"%d GiB", size / (1024 * 1024 * 1024));
	else if (size >= 1024 * 1024)
		return str(L"%d MiB", size / (1024 * 1024));
	else if (size >= 1024)
		return str(L"%d KiB", size / 1024);
	else
		return str(L"%d B", size);
}

std::wstring formatDuration(double duration)
{
	int32_t ms = (int32_t)(duration * 1000.0) % 1000;

	int32_t s = ((int32_t)duration) % 60;
	int32_t m = ((int32_t)duration) / 60;
	int32_t h = m / 60;

	m = m % 60;

	if (h > 0)	
		return str(L"%d:%02d:%02d:%04d", h, m, s, ms);
	else if (m > 0 || s > 0)
		return str(L"%02d:%02d:%04d", m, s, ms);
	else
		return str(L"%d ms", ms);
}

}
