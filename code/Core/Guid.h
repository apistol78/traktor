#ifndef traktor_Guid_H
#define traktor_Guid_H

#include <string>
#include "Core/Config.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Globally unique identifier.
 * \ingroup Core
 *
 * Globally unique identifier, with support of translating
 * back and forth from a string representation.
 */
class T_DLLCLASS Guid
{
public:
	Guid();

	/*! \brief Initialize guid from string.
	 * String must be properly formated or else
	 * the guid will be invalid.
	 *
	 * \param s Formatted guid.
	 */
	explicit Guid(const std::wstring& s);

	/*! \brief Initialize guid from bytes. */
	explicit Guid(const uint8_t data[16]);

	/*! \brief Create guid from string. */
	bool create(const std::wstring& s);

	/*! \brief Create unique guid.
	 * Create guid which is "guaranteed" to be
	 * globally unique from various parameters such as mac address etc.
	 */
	static Guid create();

	/*! \brief Format guid string. */
	std::wstring format() const;

	/*! \brief Check if guid is valid. */
	bool isValid() const;

	/*! \brief Check if guid is all zeros.
	 *
	 * Note a null guid is still a valid guid, just
	 * not representative of a guid.
	 */
	bool isNull() const;

	/*! \brief Check if guid is valid and not null. */
	bool isNotNull() const;

	/*! \brief Generate a permutation of this guid in N iterations. */
	Guid permutate(uint32_t iterations) const;

	/*! \brief Generate a permutation of this guid combined with another guid. */
	Guid permutate(const Guid& seed) const;

	/*! \brief Convert guid into 16 bytes. */
	operator const uint8_t* () const;

	/*! \brief Equal compare. */
	bool operator == (const Guid& r) const;

	/*! \brief Not equal compare. */
	bool operator != (const Guid& r) const;

	/*! \brief Less than compare. */
	bool operator < (const Guid& r) const;

	/*! \brief Greater than compare. */
	bool operator > (const Guid& r) const;

private:
	uint8_t m_data[16];	
	bool m_valid;
};

}

#endif	// traktor_Guid_H
