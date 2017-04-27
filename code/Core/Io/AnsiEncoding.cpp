/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/AnsiEncoding.h"

namespace traktor
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.AnsiEncoding", AnsiEncoding, IEncoding)

int AnsiEncoding::translate(const wchar_t* chars, int count, uint8_t* out) const
{
	int j = 0;
	for (int i = 0; i < count; ++i)
	{
		if (chars[i] <= 0xff)
			out[j++] = uint8_t(chars[i] & 0xff);
	}
	return j;
}

int AnsiEncoding::translate(const uint8_t in[MaxEncodingSize], int count, wchar_t& out) const
{
	T_ASSERT (count > 0);

	out = wchar_t(in[0]);
	return 1;
}

}
