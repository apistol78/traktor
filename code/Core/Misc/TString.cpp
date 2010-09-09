#include "Core/Io/IEncoding.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/TString.h"

namespace traktor
{

std::wstring mbstows(const IEncoding& encoding, const std::string& s)
{
	StringOutputStream ss;
	uint8_t buf[IEncoding::MaxEncodingSize];
	wchar_t ec;
	int32_t r;

	const char* cs = s.c_str();
	for (uint32_t i = 0; i < s.length(); )
	{
		uint32_t nb = std::min(sizeof_array(buf), s.length() - i);

		std::memcpy(buf, &cs[i], nb);
		if ((r = encoding.translate(buf, nb, ec)) < 0)
			break;

		ss << ec;
		i += r;
	}

	return ss.str();
}

std::string wstombs(const IEncoding& encoding, const std::wstring& s)
{
	uint8_t buf[IEncoding::MaxEncodingSize];
	std::string out;
	
	for (uint32_t i = 0; i < s.length(); ++i)
	{
		int nbuf = encoding.translate(&s[i], 1, buf);
		if (nbuf <= 0)
			continue;
			
		out += std::string(buf, buf + nbuf);
	}
	
	return out;
}

}
