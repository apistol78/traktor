#ifndef traktor_TString_H
#define traktor_TString_H

#if defined(_PS3)
#	include <calloca>
#endif
#if defined(_XBOX)
#	include <malloc.h>
#endif
#include <cstdlib>
#include <string>
#include <vector>
#include "Core/Config.h"
#if defined(__ANDROID__)
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
#else
	typedef std::string tstring;
#endif

/*! \brief Translate multibyte encoding into wide string.
 *
 * \param encoding Encoding object.
 * \param s Multibyte string.
 * \return Wide string (UTF-16 or UTF-32 encoded).
 */
std::wstring T_DLLCLASS mbstows(const IEncoding& encoding, const std::string& s);

/*! \brief Translate wide string to multibyte string using encoding.
 *
 * \param encoding Encoding object.
 * \param s Wide string.
 * \return Multibyte encoded string.
 */
std::string T_DLLCLASS wstombs(const IEncoding& encoding, const std::wstring& s);

inline
std::wstring mbstows(const std::string& mbs)
{
#if !defined(__ANDROID__)
	if (!mbs.empty())
	{
		wchar_t* buf = (wchar_t*)alloca((mbs.length() + 1) * sizeof(wchar_t));
		if (buf)
		{
			std::mbstowcs(buf, mbs.c_str(), mbs.length());
			return std::wstring(buf, mbs.length());
		}
	}
	return std::wstring();
#else
	return mbstows(Utf8Encoding(), mbs);
#endif
}

inline
std::string wstombs(const std::wstring& ws)
{
#if !defined(__ANDROID__)
	if (!ws.empty())
	{
		char* buf = (char*)alloca((ws.length() + 1) * sizeof(char));
		if (buf)
		{
			std::wcstombs(buf, ws.c_str(), ws.length());
			return std::string(buf, ws.length());
		}
	}
	return std::string();
#else
	return wstombs(Utf8Encoding(), ws);
#endif
}

#if defined(_UNICODE)
inline tstring mbstots(const std::string& mbs) { return mbstows(mbs); }
#else
inline const tstring& mbstots(const std::string& mbs) { return mbs; }
#endif

#if defined(_UNICODE)
inline const tstring& wstots(const std::wstring& ws) { return ws; }
#else
inline tstring wstots(const std::wstring& ws) { return wstombs(ws); }
#endif

#if defined(_UNICODE)
inline const std::wstring& tstows(const tstring& ts) { return ts; }
#else
inline std::wstring tstows(const tstring& ts) { return mbstows(ts); }
#endif

#if defined(_UNICODE)
inline std::string tstombs(const tstring& ts) { return wstombs(ts); }
#else
inline const std::string& tstombs(const tstring& ts) { return ts; }
#endif

#if defined(_MSC_VER)
#	pragma warning ( pop )
#endif

//@}

}

#endif	// traktor_TString_H
