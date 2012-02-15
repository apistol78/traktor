#ifndef Casts_H
#define Casts_H

#include "Condition.h"

class Casts : public Condition
{
	T_RTTI_CLASS;

public:
	virtual void check(const traktor::Path& fileName, const Source& source, bool isHeader, traktor::OutputStream& report) const;
};

#endif	// Casts_H
