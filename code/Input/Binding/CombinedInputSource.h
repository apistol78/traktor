#ifndef traktor_input_CombinedInputSource_H
#define traktor_input_CombinedInputSource_H

#include <list>
#include "Input/Binding/IInputSource.h"

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

class T_DLLCLASS CombinedInputSource : public IInputSource
{
	T_RTTI_CLASS;

public:
	CombinedInputSource(IInputSource* source1, IInputSource* source2);

	virtual std::wstring getDescription() const;
	
	virtual float read(InputSystem* inputSystem, float T, float dT);
	
private:
	Ref< IInputSource > m_source1;
	Ref< IInputSource > m_source2;
};

	}
}

#endif	// traktor_input_CombinedInputSource_H
