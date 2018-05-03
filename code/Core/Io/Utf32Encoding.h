/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Utf32Encoding_H
#define traktor_Utf32Encoding_H

#include "Core/Io/IEncoding.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief UTF-32 encoding.
 * \ingroup Core
 */
class T_DLLCLASS Utf32Encoding : public IEncoding
{
	T_RTTI_CLASS;

public:
	Utf32Encoding() {}

	virtual int translate(const wchar_t* chars, int count, uint8_t* out) const T_OVERRIDE T_FINAL;

	virtual int translate(const uint8_t in[MaxEncodingSize], int count, wchar_t& out) const T_OVERRIDE T_FINAL;
};

}

#endif	// traktor_Utf32Encoding_H
