#ifndef traktor_online_CreateTrophyContextTask_H
#define traktor_online_CreateTrophyContextTask_H

#include <np.h>
#include "Online/Psn/ISaveGameQueueTask.h"

namespace traktor
{

class ISerializable;

	namespace online
	{

class CreateTrophyContextTask : public ISaveGameQueueTask
{
	T_RTTI_CLASS;

public:
	static Ref< ISaveGameQueueTask > create(SceNpTrophyContext trophyContext, SceNpTrophyHandle trophyHandle);

	virtual bool execute();

private:
	SceNpTrophyContext m_trophyContext;
	SceNpTrophyHandle m_trophyHandle;

	static int callbackTrophyStatus(SceNpTrophyContext context, SceNpTrophyStatus status, int completed, int total, void *arg);
};

	}
}

#endif	// traktor_online_CreateTrophyContextTask_H
