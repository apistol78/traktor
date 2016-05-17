#ifndef traktor_CaseTransform_H
#define traktor_CaseTransform_H

#include "UnitTest/Case.h"

namespace traktor
{

class CaseTransform : public Case
{
public:
	virtual void run() T_OVERRIDE T_FINAL;
};

}

#endif	// traktor_CaseTransform_H
