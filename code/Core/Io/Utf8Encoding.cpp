/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/Utf8Encoding.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Utf8Encoding", Utf8Encoding, IEncoding);

int Utf8Encoding::translate(const wchar_t* chars, int count, uint8_t* out) const
{
	int j = 0;
	for (int i = 0; i < count; ++i)
	{
		wchar_t ch = chars[i];

		if (ch <= 0x7f)
		{
			// 0xxxxxxx
			out[j++] = uint8_t(ch);
			continue;
		}

		if (ch <= 0x7ff)
		{
			// 110xxxxx 10xxxxxx
			out[j++] = 0xc0 | uint8_t(ch >> 6);
			out[j++] = 0x80 | uint8_t(ch & 0x3f);
			continue;
		}

		if (ch <= 0xffff)
		{
			// 1110xxxx 10xxxxxx 10xxxxxx
			out[j++] = 0xe0 | uint8_t(ch >> 12);
			out[j++] = 0x80 | uint8_t(ch >> 6);
			out[j++] = 0x80 | uint8_t(ch & 0x3f);
			continue;
		}

#if !defined(_WIN32) && !defined(_PS3)	// Windows use UCS-2 as wide characters.

		if (ch <= 0x1fffff)
		{
			// 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
			out[j++] = 0xf0 | uint8_t(ch >> 18);
			out[j++] = 0x80 | uint8_t(ch >> 12);
			out[j++] = 0x80 | uint8_t(ch >> 6);
			out[j++] = 0x80 | uint8_t(ch & 0x3f);
			continue;
		}

		if (ch <= 0x03ffffff)
		{
			// 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
			out[j++] = 0xf8 | uint8_t(ch >> 24);
			out[j++] = 0x80 | uint8_t(ch >> 18);
			out[j++] = 0x80 | uint8_t(ch >> 12);
			out[j++] = 0x80 | uint8_t(ch >> 6);
			out[j++] = 0x80 | uint8_t(ch & 0x3f);
			continue;
		}

		if (ch <= 0x7fffffff)
		{
			// 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
			out[j++] = 0xfc | uint8_t(ch >> 30);
			out[j++] = 0x80 | uint8_t(ch >> 24);
			out[j++] = 0x80 | uint8_t(ch >> 18);
			out[j++] = 0x80 | uint8_t(ch >> 12);
			out[j++] = 0x80 | uint8_t(ch >> 6);
			out[j++] = 0x80 | uint8_t(ch & 0x3f);
			continue;
		}

#endif
	}
	return j;
}

int Utf8Encoding::translate(const uint8_t in[MaxEncodingSize], int count, wchar_t& out) const
{
	out = 0;

	if ((in[0] & 0xe0) == 0xc0)
	{
		// 110xxxxx 10xxxxxx
		out = (wchar_t(in[0] & 0x1f) << 6) | wchar_t(in[1] & 0x3f);
		return 2;
	}
	else if ((in[0] & 0xf0) == 0xe0)
	{
		// 1110xxxx 10xxxxxx 10xxxxxx
		out = (wchar_t(in[0] & 0x0f) << 12) | (wchar_t(in[1] & 0x3f) << 6) | wchar_t(in[2] & 0x3f);
		return 3;
	}
#if !defined(_WIN32) && !defined(_PS3)	// Windows use UCS-2 as wide characters.
	else if ((in[0] & 0xf8) == 0xf0)
	{
		// 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx	
		out = (wchar_t(in[0] & 0x0f) << 18) | (wchar_t(in[1] & 0x3f) << 12) | (wchar_t(in[2] & 0x3f) << 6) | wchar_t(in[3] & 0x3f);
		return 4;
	}
	else if ((in[0] & 0xfc) == 0xf8)
	{
		// 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		out = (wchar_t(in[0] & 0x0f) << 24) | (wchar_t(in[1] & 0x3f) << 18) | (wchar_t(in[2] & 0x3f) << 12) | (wchar_t(in[3] & 0x3f) << 6) | wchar_t(in[4] & 0x3f);
		return 5;
	}
	else if ((in[0] & 0xfe) == 0xfc)
	{
		// 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		out = (wchar_t(in[0] & 0x0f) << 30) | (wchar_t(in[1] & 0x3f) << 24) | (wchar_t(in[2] & 0x3f) << 18) | (wchar_t(in[3] & 0x3f) << 12) | (wchar_t(in[4] & 0x3f) << 6) | wchar_t(in[5] & 0x3f);
		return 6;
	}
#endif

	T_ASSERT ((in[0] & 0x80) == 0x00);
	out = wchar_t(in[0]);
	return 1;
}

}
