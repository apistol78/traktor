#ifndef traktor_input_InputState_H
#define traktor_input_InputState_H

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
	
class InputStateData;
class InputValueSet;

class InputState : public Object
{
	T_RTTI_CLASS;

public:
	InputState(const InputStateData* data);

	void update(const InputValueSet& valueSet);
	
private:
	Ref< const InputStateData > m_data;
	float m_previousValue;
	float m_currentValue;
};

	}
}

#endif	// traktor_input_InputState_H
