/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_Adler32_H
#define traktor_Adler32_H

#include "Core/Misc/IHash.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/* \brief Adler32 checksum.
 * \ingroup Core
 */
class T_DLLCLASS Adler32 : public IHash
{
	T_RTTI_CLASS;

public:
	Adler32();

	virtual void begin() T_OVERRIDE T_FINAL;

	virtual void feed(const void* buffer, uint64_t bufferSize) T_OVERRIDE T_FINAL;

	virtual void end() T_OVERRIDE T_FINAL;

	uint32_t get() const;

	template < typename T >
	void feed(const T& value)
	{
		feed(&value, sizeof(value));
	}

	void feed(const std::wstring& value)
	{
		if (!value.empty())
			feed(value.c_str(), uint64_t(value.length()));
	}

private:
	uint32_t m_A;
	uint32_t m_B;
	uint32_t m_feed;
};

}

#endif	// traktor_Adler32_H
