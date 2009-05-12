#ifndef traktor_MD5_H
#define traktor_MD5_H

#include <string>
#include "Core/Serialization/Serializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief MD5 checksum.
 * \ingroup Core
 */
class T_DLLCLASS MD5 : public Serializable
{
	T_RTTI_CLASS(MD5)

public:
	MD5();

	void begin();

	void feed(const void* buffer, uint32_t bufferSize);

	void end();

	const uint32_t* get() const;

	std::wstring format() const;

	bool operator == (const MD5& md5) const;

	bool operator != (const MD5& md5) const;

	bool operator < (const MD5& md5) const;

	virtual bool serialize(Serializer& s);

private:
	uint8_t m_buffer[64];
	uint32_t m_count[2];
	uint32_t m_md5[4];

	void transform(const uint8_t block[64]);
};

}

#endif	// traktor_MD5_H
