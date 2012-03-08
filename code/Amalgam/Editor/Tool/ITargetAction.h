#ifndef traktor_amalgam_ITargetAction_H
#define traktor_amalgam_ITargetAction_H

#include "Core/Object.h"

namespace traktor
{
	namespace amalgam
	{

class ITargetAction : public Object
{
	T_RTTI_CLASS;

public:
	struct IProgressListener : public IRefCount
	{
		virtual void notifyTargetActionProgress(int32_t currentStep, int32_t maxStep) = 0;
	};

	virtual bool execute(IProgressListener* progressListener) = 0;
};

	}
}

#endif	// traktor_amalgam_ITargetAction_H
