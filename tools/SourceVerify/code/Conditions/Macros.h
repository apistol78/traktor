#ifndef Macros_H
#define Macros_H

#include "Condition.h"

class Macros : public Condition
{
	T_RTTI_CLASS(Macros)

public:
	virtual void check(const Source& source, bool isHeader, traktor::OutputStream& report) const;
};

#endif	// Macros_H
