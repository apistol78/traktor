#ifndef Condition_H
#define Condition_H

#include <vector>
#include <string>
#include <Core/Object.h>
#include <Core/Io/OutputStream.h>

class Condition : public traktor::Object
{
	T_RTTI_CLASS(Condition)

public:
	virtual void check(const std::vector< std::wstring >& lines, bool isHeader, traktor::OutputStream& report) const = 0;
};

#endif	// Condition_H
