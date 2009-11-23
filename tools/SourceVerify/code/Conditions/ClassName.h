#ifndef ClassName_H
#define ClassName_H

#include "Condition.h"

class ClassName : public Condition
{
	T_RTTI_CLASS;

public:
	virtual void check(const traktor::Path& fileName, const Source& source, bool isHeader, traktor::OutputStream& report) const;
};

#endif	// ClassName_H
