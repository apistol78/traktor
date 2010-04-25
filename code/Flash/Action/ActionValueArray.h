#ifndef traktor_flash_ActionValueArray_H
#define traktor_flash_ActionValueArray_H

#include "Core/Misc/AutoPtr.h"
#include "Flash/Action/ActionValue.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

/*! \brief ActionScript mutable value array.
 * \ingroup Flash
 */
class T_DLLCLASS ActionValueArray : public Object
{
	T_RTTI_CLASS;

public:
	ActionValueArray(uint32_t size)
	:	m_values(new ActionValue [size])
	,	m_size(size)
	{
	}
	
	inline uint32_t size() const
	{
		return m_size;
	}
	
	inline ActionValue& operator [] (uint32_t index)
	{
		T_ASSERT (index < m_size);
		return m_values[index];
	}
	
	inline const ActionValue& operator [] (uint32_t index) const
	{
		T_ASSERT (index < m_size);
		return m_values[index];
	}
	
private:
	AutoArrayPtr< ActionValue > m_values;
	uint32_t m_size;
};
	
	}
}

#endif	// traktor_flash_ActionValueArray_H
