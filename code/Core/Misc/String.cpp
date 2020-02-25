#include <algorithm>
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

std::string transform(const std::string& str, int (*func)(int c))
{
	std::string cpy = str;
	std::transform(cpy.begin(), cpy.end(), cpy.begin(), func);
	return cpy;
}

std::wstring transform(const std::wstring& str, int (*func)(int c))
{
	std::wstring cpy = str;
	std::transform(cpy.begin(), cpy.end(), cpy.begin(), func);
	return cpy;
}

std::string toUpper(const std::string& str)
{
	return transform(str, std::toupper);
}

std::wstring toUpper(const std::wstring& str)
{
	return transform(str, std::toupper);
}

std::string toLower(const std::string& str)
{
	return transform(str, std::tolower);
}

std::wstring toLower(const std::wstring& str)
{
	return transform(str, std::tolower);
}

bool startsWith(const std::string& str, const std::string& start)
{
	if (str.length() < start.length())
		return false;
	if (start.length() <= 0)
		return true;
	return (bool)(str.substr(0, start.length()) == start);
}

bool startsWith(const std::wstring& str, const std::wstring& start)
{
	if (str.length() < start.length())
		return false;
	if (start.length() <= 0)
		return true;
	return (bool)(str.substr(0, start.length()) == start);
}

bool endsWith(const std::string& str, const std::string& end)
{
	if (str.length() < end.length())
		return false;
	if (end.length() <= 0)
		return true;
	return (bool)(str.substr(str.length() - end.length(), end.length()) == end);
}

bool endsWith(const std::wstring& str, const std::wstring& end)
{
	if (str.length() < end.length())
		return false;
	if (end.length() <= 0)
		return true;
	return (bool)(str.substr(str.length() - end.length(), end.length()) == end);
}

int compareIgnoreCase(const std::string& a, const std::string& b)
{
	return toLower(a).compare(toLower(b));
}

int compareIgnoreCase(const std::wstring& a, const std::wstring& b)
{
	return toLower(a).compare(toLower(b));
}

std::wstring ltrim(const std::wstring& a, const std::wstring& ws)
{
	size_t i = a.find_first_not_of(ws);
	if (i == std::wstring::npos)
		return L"";
	return a.substr(i);
}

std::wstring rtrim(const std::wstring& a, const std::wstring& ws)
{
	size_t i = a.find_last_not_of(ws);
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
	size_t p = str.find_first_of(delim);
	if (p == str.npos)
		return false;

	outLeft = str.substr(0, p);
	outRight = str.substr(p + 1);
	return true;
}

}