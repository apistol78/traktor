#ifndef StatementDisposition_H
#define StatementDisposition_H

#include "Condition.h"

class StatementDisposition : public Condition
{
	T_RTTI_CLASS(StatementDisposition)

public:
	virtual void check(const traktor::Path& fileName, const Source& source, bool isHeader, traktor::OutputStream& report) const;
};

#endif	// StatementDisposition_H
