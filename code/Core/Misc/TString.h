#ifndef traktor_TString_H
#define traktor_TString_H

#include <cstdlib>
#include <string>
#include <vector>
#include "Core/Config.h"

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

	inline
	std::wstring mbstows(const std::string& mbs)
	{
		if (!mbs.empty())
		{
			std::vector< wchar_t > buf(mbs.length() + 1);
			std::mbstowcs(&buf[0], mbs.c_str(), mbs.length());
			return std::wstring(&buf[0], mbs.length());
		}
		else
			return std::wstring();
	}

	inline
	std::string wstombs(const std::wstring& ws)
	{
		if (!ws.empty())
		{
			std::vector< char > buf(ws.length() + 1);
			std::wcstombs(&buf[0], ws.c_str(), ws.length());
			return std::string(&buf[0], ws.length());
		}
		else
			return std::string();
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

#if defined(_MSC_VER)
#	pragma warning ( pop )
#endif

//@}

}

#endif	// traktor_TString_H
