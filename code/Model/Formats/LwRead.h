#ifndef traktor_model_LwRead_H
#define traktor_model_LwRead_H

#include <sstream>
#include "Core/Io/IStream.h"
#include "Core/Math/Vector4.h"
#include "Core/Misc/Endian.h"
#include "Core/Misc/TString.h"

namespace traktor
{
	namespace model
	{

template < typename T >
inline T fromBigEndian(T value)
{
#if defined(T_LITTLE_ENDIAN)
	swap8in32(value);
#endif
	return value;
}

template < typename T >
inline bool lwRead(IStream* stream, T& out)
{
	if (stream->read(&out, sizeof(T)) == sizeof(T))
	{
		out = fromBigEndian< T >(out);
		return true;
	}
	else
		return false;
}

template < typename T >
inline bool lwRead(IStream* stream, T* out, int32_t count)
{
	for (int32_t i  = 0; i < count; ++i)
	{
		if (!lwRead< T >(stream, out[i]))
			return false;
	}
	return true;
}

template < >
inline bool lwRead< std::wstring >(IStream* stream, std::wstring& out)
{
	std::stringstream ss;

	do
	{
		char ch;
		if (stream->read(&ch, sizeof(ch)) != sizeof(ch))
			return false;

		if (ch == 0)
			break;

		ss << ch;
	}
	while (true);

	out = mbstows(ss.str());

	if ((out.length() & 1) == 0)
	{
		char ch;
		if (stream->read(&ch, sizeof(ch)) != sizeof(ch))
			return false;
	}
		
	return true;
}

template < >
inline bool lwRead< Vector4 >(IStream* stream, Vector4& out)
{
	float e[3];

	if (!lwRead< float >(stream, e[0]))
		return false;
	if (!lwRead< float >(stream, e[1]))
		return false;
	if (!lwRead< float >(stream, e[2]))
		return false;

	out = Vector4(e[0], e[1], e[2], 1.0f);
	return true;
}

inline bool lwReadIndex(IStream* stream, uint32_t& out)
{
	uint16_t vh;
	if (!lwRead< uint16_t >(stream, vh))
		return false;

	if ((vh & 0xff00) == 0xff00)
	{
		uint16_t vl;
		if (!lwRead< uint16_t >(stream, vl))
			return false;

		out = ((vh & 0xff) << 16) | vl;
	}
	else
		out = vh;

	return true;
}

	}
}

#endif	// traktor_model_LwRead_H
