/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/Utf32Encoding.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Utf32Encoding", Utf32Encoding, IEncoding)

int Utf32Encoding::translate(const wchar_t* chars, int count, uint8_t* out) const
{
	int j = 0;
	for (int i = 0; i < count; ++i)
	{
		wchar_t ch = chars[i];
		out[j++] = uint8_t(ch);
		out[j++] = uint8_t(ch >> 8);
#if !defined(_WIN32)
		out[j++] = uint8_t(ch >> 16);
		out[j++] = uint8_t(ch >> 24);
#else
		out[j++] = 0;
		out[j++] = 0;
#endif
	}
	return j;
}

int Utf32Encoding::translate(const uint8_t in[MaxEncodingSize], int count, wchar_t& out) const
{
#if !defined(_WIN32)
	out = wchar_t(in[3] << 24) | wchar_t(in[2] << 16) | wchar_t(in[1] << 8) | in[0];
#else
	out = wchar_t(in[1] << 8) | in[0];
#endif
	return 4;
}

}
