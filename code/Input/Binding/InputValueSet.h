#ifndef traktor_input_InputValueSet_H
#define traktor_input_InputValueSet_H

#include <map>
#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace input
	{
	
/*! \brief
 * \ingroup Input
 */
class T_DLLCLASS InputValueSet : public Object
{
	T_RTTI_CLASS;

public:
	void set(const std::wstring& valueId, float value);
	
	float get(const std::wstring& valueId) const;
	
private:
	std::map< std::wstring, float > m_valueMap;
};
	
	}
}

#endif	// traktor_input_InputValueSet_H
