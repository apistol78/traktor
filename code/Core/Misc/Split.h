/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Split_H
#define traktor_Split_H

#include <vector>
#include "Core/Config.h"
#include "Core/Misc/TString.h"

namespace traktor
{

template < typename String, typename Value >
struct ConvertPolicy
{
};

template < >
struct ConvertPolicy< std::string, std::string >
{
	static const std::string& convert(const std::string& piece) {
		return piece;
	}
};

template < >
struct ConvertPolicy< std::wstring, std::wstring >
{
	static const std::wstring& convert(const std::wstring& piece) {
		return piece;
	}
};

template < >
struct ConvertPolicy< std::string, int32_t >
{
	static int32_t convert(const std::string& piece) {
		return int32_t(std::atoi(piece.c_str()));
	}
};

template < >
struct ConvertPolicy< std::wstring, int32_t >
{
	static int32_t convert(const std::wstring& piece) {
#if defined(WIN32)
		return int32_t(_wtoi(piece.c_str()));
#else
		return int32_t(std::wcstol(piece.c_str(), 0, 0));
#endif
	}
};

template < >
struct ConvertPolicy< std::string, uint32_t >
{
	static uint32_t convert(const std::string& piece) {
		return uint32_t(std::strtoul(piece.c_str(), 0, 10));
	}
};

template < >
struct ConvertPolicy< std::wstring, uint32_t >
{
	static uint32_t convert(const std::wstring& piece) {
		return uint32_t(std::wcstoul(piece.c_str(), 0, 10));
	}
};

template < >
struct ConvertPolicy< std::string, float >
{
	static float convert(const std::string& piece) {
		return float(std::atof(piece.c_str()));
	}
};

template < >
struct ConvertPolicy< std::wstring, float >
{
	static float convert(const std::wstring& piece) {
#if defined(WIN32)
		return float(_wtof(piece.c_str()));
#else
		return float(std::wcstod(piece.c_str(), 0));
#endif
	}
};

/*! \brief String splitter.
 * \ingroup Core
 *
 * Split< std::wstring >::any(L"A,B,C", ",", out);
 * Split< std::wstring >::word(L"A,B,C", ",", out);
 */
template < typename String, typename Value = String, typename _CP = ConvertPolicy< String, Value > >
struct Split
{
	template < typename ContainerType >
	static size_t any(const String& str, const String& delimiters, ContainerType& out, bool keepEmpty = false, size_t max = ~0UL)
	{
		typename String::size_type n = str.length();
		typename String::size_type start, stop;

		start = str.find_first_not_of(delimiters);
		while (start < n)
		{
			stop = str.find_first_of(delimiters, start);
			if (stop > n)
				stop = n;

			if (stop > start || keepEmpty)
				out.push_back(_CP::convert(str.substr(start, stop - start)));
			start = stop + 1;

			if (out.size() >= max)
			{
				if (start < n)
					out.push_back(_CP::convert(str.substr(start)));
				break;
			}
		}

		if (n == 0 && keepEmpty)
			out.push_back(_CP::convert(str));

		return out.size();
	}

	template < typename ContainerType >
	static size_t word(const String& str, const String& delimiter, ContainerType& out, size_t max = ~0UL)
	{
		typename String::size_type n = str.length();
		typename String::size_type x = delimiter.length();
		typename String::size_type start, stop;

		if (n == 0)
			return 0;

		start = 0;
		while (start < n - x)
		{
			stop = str.find(delimiter, start);
			if (stop > n)
				stop = n;

			if (stop > start)
				out.push_back(_CP::convert(str.substr(start, stop - start)));

			start = stop + x;

			if (out.size() >= max)
			{
				if (start < n)
					out.push_back(_CP::convert(str.substr(start)));
				break;
			}
		}

		return out.size();
	}
};

}

#endif	// traktor_Split_H
