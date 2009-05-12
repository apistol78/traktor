#ifndef traktor_flash_ActionDictionary_H
#define traktor_flash_ActionDictionary_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

/*! \brief ActionScript constant string dictionary.
 * \ingroup Flash
 */
class T_DLLCLASS ActionDictionary : public Object
{
	T_RTTI_CLASS(ActionDictionary)

public:
	ActionDictionary(uint16_t tableSize, const char* tableFirstEntry);

	inline const char* get(uint16_t index) const
	{
		T_ASSERT (index < m_table.size());
		return m_table[index];
	}

private:
	std::vector< const char* > m_table;
};

	}
}

#endif	// traktor_flash_ActionDictionary_H
