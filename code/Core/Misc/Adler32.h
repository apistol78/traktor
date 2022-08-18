#pragma once

#include <string>
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

/* Adler32 checksum.
 * \ingroup Core
 */
class T_DLLCLASS Adler32 : public IHash
{
	T_RTTI_CLASS;

public:
	virtual void begin() override final;

	virtual void feed(const void* buffer, uint64_t bufferSize) override final;

	virtual void end() override final;

	uint32_t get() const;

	template < typename T >
	void feed(const T& value)
	{
		feed(&value, sizeof(value));
	}

	void feed(const std::wstring& value)
	{
		if (!value.empty())
			feed(value.c_str(), (uint64_t)(value.length() * sizeof(wchar_t)));
	}

private:
	uint32_t m_A = 0;
	uint32_t m_B = 0;
	uint32_t m_feed = 0;
};

}

