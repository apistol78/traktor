#ifndef traktor_sound_IGrainFacade_H
#define traktor_sound_IGrainFacade_H

#include "Core/Object.h"
#include "Core/RefArray.h"

namespace traktor
{
	namespace sound
	{

class IGrain;

class IGrainFacade : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool canHaveChildren() const = 0;

	virtual bool addChild(IGrain* parentGrain, IGrain* childGrain) = 0;

	virtual bool removeChild(IGrain* parentGrain, IGrain* childGrain) = 0;

	virtual bool getChildren(IGrain* grain, RefArray< IGrain >& outChildren) = 0;
};

	}
}

#endif	// traktor_sound_IGrainFacade_H
