#ifndef traktor_sound_RepeatGrainFacade_H
#define traktor_sound_RepeatGrainFacade_H

#include "Sound/Editor/Resound/IGrainFacade.h"

namespace traktor
{
	namespace sound
	{

class RepeatGrainFacade : public IGrainFacade
{
	T_RTTI_CLASS;

public:
	virtual bool canHaveChildren() const;

	virtual bool addChild(IGrain* parentGrain, IGrain* childGrain);

	virtual bool removeChild(IGrain* parentGrain, IGrain* childGrain);

	virtual bool getChildren(IGrain* grain, RefArray< IGrain >& outChildren);
};

	}
}

#endif	// traktor_sound_RepeatGrainFacade_H
