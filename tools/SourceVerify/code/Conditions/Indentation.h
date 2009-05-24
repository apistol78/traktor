#ifndef Indentation_H
#define Indentation_H

#include "Condition.h"

class Indentation : public Condition
{
	T_RTTI_CLASS(Indentation)

public:
	virtual void check(const std::vector< std::wstring >& lines, bool isHeader, traktor::OutputStream& report) const;
};

#endif	// Indentation_H
