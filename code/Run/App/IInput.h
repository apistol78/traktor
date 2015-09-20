#ifndef traktor_run_IInput_H
#define traktor_run_IInput_H

#include "Core/Object.h"

namespace traktor
{
	namespace run
	{

class IInput : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool endOfFile() = 0;

	virtual std::wstring readChar() = 0;

	virtual std::wstring readLn() = 0;
};

	}
}

#endif	// traktor_run_IInput_H
