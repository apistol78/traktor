#ifndef traktor_CasePath_H
#define traktor_CasePath_H

#include "UnitTest/Case.h"

namespace traktor
{

class CasePath : public Case
{
public:
	virtual void run() T_OVERRIDE T_FINAL;
};

}

#endif	// traktor_CasePath_H
