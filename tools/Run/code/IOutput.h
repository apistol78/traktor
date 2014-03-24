#ifndef IOutput_H
#define IOutput_H

#include <Core/Object.h>

class IOutput : public traktor::Object
{
	T_RTTI_CLASS;

public:
	virtual void print(const std::wstring& s) = 0;

	virtual void printLn(const std::wstring& s) = 0;
};

#endif	// IOutput_H
