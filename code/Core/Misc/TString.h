/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <cstdlib>
#include <string>
#include <vector>
#include "Core/Config.h"
#if defined(__ANDROID__) || defined(__LINUX__) || defined(__RPI__)
#	include "Core/Io/Utf8Encoding.h"
#endif

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IEncoding;

#if defined(_MSC_VER)
#	pragma warning ( push )
#	pragma warning ( disable : 4996 )	// Do not complain about non-secure functions.
#endif

/*! \ingroup Core */
//@{

#if defined(_UNICODE)
	typedef std::wstring tstring;
	typedef std::wstring_view tstring_view;
#else
	typedef std::string tstring;
	typedef std::string_view tstring_view;
#endif

/*! Translate multibyte encoding into wide string.
 *
 * \param encoding Encoding object.
 * \param s Multibyte string.
 * \return Wide string (UTF-16 or UTF-32 encoded).
 */
std::wstring T_DLLCLASS mbstows(const IEncoding& encoding, const std::string_view& s);

/*! Translate wide string to multibyte string using encoding.
 *
 * \param encoding Encoding object.
 * \param s Wide string.
 * \return Multibyte encoded string.
 */
std::string T_DLLCLASS wstombs(const IEncoding& encoding, const std::wstring_view& s);

inline
std::wstring mbstows(const std::string_view& mbs)
{
#if !defined(__ANDROID__) && !defined(__LINUX__) && !defined(__RPI__)
	if (!mbs.empty())
	{
#	if defined(_WIN32)
		wchar_t* buf = (wchar_t*)_malloca((mbs.length() + 1) * sizeof(wchar_t));
#	else
		wchar_t* buf = (wchar_t*)alloca((mbs.length() + 1) * sizeof(wchar_t));
#	endif
		if (buf)
		{
			std::mbstowcs(buf, mbs.data(), mbs.length());
			return std::wstring(buf, mbs.length());
		}
	}
	return std::wstring();
#else
	return mbstows(Utf8Encoding(), mbs);
#endif
}

inline
std::string wstombs(const std::wstring_view& ws)
{
#if !defined(__ANDROID__) && !defined(__LINUX__) && !defined(__RPI__)
	if (!ws.empty())
	{
#	if defined(_WIN32)
		char* buf = (char*)_malloca((ws.length() + 1) * sizeof(char));
#	else
		char* buf = (char*)alloca((ws.length() + 1) * sizeof(char));
#	endif
		if (buf)
		{
			std::wcstombs(buf, ws.data(), ws.length());
			return std::string(buf, ws.length());
		}
	}
	return std::string();
#else
	return wstombs(Utf8Encoding(), ws);
#endif
}

#if defined(_UNICODE)
inline tstring mbstots(const std::string_view& mbs) { return mbstows(mbs); }
#else
inline tstring mbstots(const std::string_view& mbs) { return tstring(mbs); }
#endif

#if defined(_UNICODE)
inline tstring wstots(const std::wstring_view& ws) { return tstring(ws); }
#else
inline tstring wstots(const std::wstring_view& ws) { return wstombs(ws); }
#endif

#if defined(_UNICODE)
inline std::wstring tstows(const tstring_view& ts) { return std::wstring(ts); }
#else
inline std::wstring tstows(const tstring_view& ts) { return mbstows(ts); }
#endif

#if defined(_UNICODE)
inline std::string tstombs(const tstring_view& ts) { return wstombs(ts); }
#else
inline std::string tstombs(const tstring_view& ts) { return std::string(ts); }
#endif

#if defined(_MSC_VER)
#	pragma warning ( pop )
#endif

//@}

}

