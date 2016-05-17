#ifndef traktor_CaseMeta_H
#define traktor_CaseMeta_H

#include "UnitTest/Case.h"

namespace traktor
{

class CaseMeta : public Case
{
public:
	virtual void run() T_OVERRIDE T_FINAL;
};

}

#endif	// traktor_CaseMeta_H
