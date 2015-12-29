#ifndef traktor_SHA1_H
#define traktor_SHA1_H

#include <string>
#include "Core/Ref.h"
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

/*! \brief SHA-1 hash.
 * \ingroup Core
 */
class T_DLLCLASS SHA1 : public Object
{
	T_RTTI_CLASS;

public:
	SHA1();

	/*! \brief Begin feeding data for SHA1 checksum calculation. */
	void begin();

	/*! \brief Feed data to SHA1 checksum calculation.
	 *
	 * \param buffer Pointer to data.
	 * \param bufferSize Amount of data in bytes.
	 */
	void feed(const void* buffer, uint32_t bufferSize);

	/*! \brief End feeding data for SHA1 checksum calculation. */
	void end();

private:
	static const size_t HASH_LENGTH = 20;
	static const size_t BLOCK_LENGTH = 64;

	struct sha1nfo {
		uint32_t buffer[BLOCK_LENGTH/4];
		uint32_t state[HASH_LENGTH/4];
		uint32_t byteCount;
		uint8_t bufferOffset;
		uint8_t keyBuffer[BLOCK_LENGTH];
		uint8_t innerHash[HASH_LENGTH];
	};

	static void sha1_init(sha1nfo *s);
	static void sha1_hashBlock(sha1nfo *s);
	static void sha1_addUncounted(sha1nfo *s, uint8_t data);
	static void sha1_writebyte(sha1nfo *s, uint8_t data);
	static void sha1_write(sha1nfo *s, const char *data, size_t len);
	static void sha1_pad(sha1nfo *s);
	static uint8_t* sha1_result(sha1nfo *s);
	static void sha1_initHmac(sha1nfo *s, const uint8_t* key, int keyLength);
	static uint8_t* sha1_resultHmac(sha1nfo *s);

	sha1nfo m_sha1nfo;
};

}

#endif	// traktor_SHA1_H
