#ifndef traktor_input_InputValueSet_H
#define traktor_input_InputValueSet_H

#include <map>
#include "Core/Object.h"
#include "Input/Binding/InputValue.h"

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
	
class T_DLLCLASS InputValueSet : public Object
{
	T_RTTI_CLASS;

public:
	void set(const std::wstring& valueId, const InputValue& value);
	
	InputValue get(const std::wstring& valueId) const;
	
private:
	std::map< std::wstring, InputValue > m_valueMap;
};
	
	}
}

#endif	// traktor_input_InputValueSet_H
