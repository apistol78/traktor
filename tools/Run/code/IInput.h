#ifndef IInput_H
#define IInput_H

#include <Core/Object.h>

class IInput : public traktor::Object
{
	T_RTTI_CLASS;

public:
	virtual bool endOfFile() = 0;

	virtual std::wstring readChar() = 0;

	virtual std::wstring readLn() = 0;
};

#endif	// IInput_H
