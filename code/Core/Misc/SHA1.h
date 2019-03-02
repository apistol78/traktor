#pragma once

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

/*! \brief SHA-1 hash.
 * \ingroup Core
 */
class T_DLLCLASS SHA1 : public IHash
{
	T_RTTI_CLASS;

public:
	SHA1();

	virtual ~SHA1();

	bool createFromString(const std::wstring& str);

	/*! \brief Begin feeding data for SHA1 checksum calculation. */
	virtual void begin() override final;

	/*! \brief Feed data to SHA1 checksum calculation.
	 *
	 * \param buffer Pointer to data.
	 * \param bufferSize Amount of data in bytes.
	 */
	virtual void feed(const void* buffer, uint64_t bufferSize) override final;

	/*! \brief End feeding data for SHA1 checksum calculation. */
	virtual void end() override final;

	/*! \brief Format MD5 checksum as string. */
	std::wstring format() const;

private:
	void* m_sha1nfo;
};

}

