#ifndef traktor_CaseClone_H
#define traktor_CaseClone_H

#include "UnitTest/Case.h"

namespace traktor
{

class CaseClone : public Case
{
public:
	virtual void run() T_OVERRIDE T_FINAL;
};

}

#endif	// traktor_CaseClone_H
