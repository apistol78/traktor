#ifndef traktor_online_ISaveGameQueueTask_H
#define traktor_online_ISaveGameQueueTask_H

#include "Core/Object.h"

namespace traktor
{
	namespace online
	{

class ISaveGameQueueTask : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool execute() = 0;
};

	}
}

#endif	// traktor_online_ISaveGameQueueTask_H
