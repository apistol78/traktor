#ifndef IncludeGuard_H
#define IncludeGuard_H

#include "Condition.h"

class IncludeGuard : public Condition
{
	T_RTTI_CLASS(IncludeGuard)

public:
	virtual void check(const std::vector< std::wstring >& lines, bool isHeader, traktor::OutputStream& report) const;
};

#endif	// IncludeGuard_H
