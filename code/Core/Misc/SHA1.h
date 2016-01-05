#ifndef traktor_SHA1_H
#define traktor_SHA1_H

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

	virtual ~SHA1();

	/*! \brief Begin feeding data for SHA1 checksum calculation. */
	void begin();

	bool createFromString(const std::wstring& str);

	/*! \brief Feed data to SHA1 checksum calculation.
	 *
	 * \param buffer Pointer to data.
	 * \param bufferSize Amount of data in bytes.
	 */
	void feed(const void* buffer, uint32_t bufferSize);

	/*! \brief End feeding data for SHA1 checksum calculation. */
	void end();

	/*! \brief Format MD5 checksum as string. */
	std::wstring format() const;

private:
	void* m_sha1nfo;
};

}

#endif	// traktor_SHA1_H
