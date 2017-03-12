#ifndef traktor_Adler32_H
#define traktor_Adler32_H

#include "Core/Object.h"

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
class T_DLLCLASS Adler32 : public Object
{
	T_RTTI_CLASS;

public:
	Adler32();

	void begin();

	void feed(const void* buffer, uint64_t bufferSize);

	void end();

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
