#pragma once

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

/*! UTF-16 encoding.
 * \ingroup Core
 */
class T_DLLCLASS Utf16Encoding : public IEncoding
{
	T_RTTI_CLASS;

public:
	Utf16Encoding() = default;

	virtual int translate(const wchar_t* chars, int32_t count, uint8_t* out) const override final;

	virtual int translate(const uint8_t in[MaxEncodingSize], int32_t count, wchar_t& out) const override final;
};

}

