#ifndef traktor_input_InputValueSet_H
#define traktor_input_InputValueSet_H

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Input/InputTypes.h"

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
	void set(handle_t valueId, float value);
	
	float get(handle_t valueId) const;
	
private:
	SmallMap< handle_t, float > m_valueMap;
};
	
	}
}

#endif	// traktor_input_InputValueSet_H
