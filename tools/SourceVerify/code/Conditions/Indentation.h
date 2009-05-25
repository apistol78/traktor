#ifndef Indentation_H
#define Indentation_H

#include "Condition.h"

class Indentation : public Condition
{
	T_RTTI_CLASS(Indentation)

public:
	virtual void check(const Source& source, bool isHeader, traktor::OutputStream& report) const;
};

#endif	// Indentation_H
