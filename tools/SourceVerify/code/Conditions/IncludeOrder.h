#ifndef IncludeOrder_H
#define IncludeOrder_H

#include "Condition.h"

class IncludeOrder : public Condition
{
	T_RTTI_CLASS(IncludeOrder)

public:
	virtual void check(const std::vector< std::wstring >& lines, bool isHeader, traktor::OutputStream& report) const;
};

#endif	// IncludeOrder_H
