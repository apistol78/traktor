#ifndef IncludeOrder_H
#define IncludeOrder_H

#include "Condition.h"

class IncludeOrder : public Condition
{
	T_RTTI_CLASS(IncludeOrder)

public:
	virtual void check(const traktor::Path& fileName, const Source& source, bool isHeader, traktor::OutputStream& report) const;
};

#endif	// IncludeOrder_H
