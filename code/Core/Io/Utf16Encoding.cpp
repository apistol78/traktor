/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/Utf16Encoding.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.Utf16Encoding", Utf16Encoding, IEncoding);

int Utf16Encoding::translate(const wchar_t* chars, int count, uint8_t* out) const
{
	int j = 0;
	for (int i = 0; i < count; ++i)
	{
		wchar_t ch = chars[i];
		out[j++] = uint8_t(ch);
		out[j++] = uint8_t(ch >> 8);
	}
	return j;
}

int Utf16Encoding::translate(const uint8_t in[MaxEncodingSize], int count, wchar_t& out) const
{
	out = wchar_t(in[1] << 8) | in[0];
	return 2;
}

}
