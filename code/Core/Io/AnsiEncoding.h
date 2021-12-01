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

/*! ANSI encoding.
 * \ingroup Core
 */
class T_DLLCLASS AnsiEncoding : public IEncoding
{
	T_RTTI_CLASS;

public:
	virtual int32_t translate(const wchar_t* chars, int32_t count, uint8_t* out) const override final;

	virtual int32_t translate(const uint8_t in[MaxEncodingSize], int32_t count, wchar_t& out) const override final;
};

}

