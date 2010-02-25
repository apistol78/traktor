#ifndef traktor_sound_RandomGrainFacade_H
#define traktor_sound_RandomGrainFacade_H

#include "Sound/Editor/Resound/IGrainFacade.h"

namespace traktor
{
	namespace sound
	{

class RandomGrainFacade : public IGrainFacade
{
	T_RTTI_CLASS;

public:
	virtual int32_t getImage(const IGrain* grain) const;

	virtual std::wstring getText(const IGrain* grain) const;

	virtual bool canHaveChildren() const;

	virtual bool addChild(IGrain* parentGrain, IGrain* childGrain);

	virtual bool removeChild(IGrain* parentGrain, IGrain* childGrain);

	virtual bool getChildren(IGrain* grain, RefArray< IGrain >& outChildren);
};

	}
}

#endif	// traktor_sound_RandomGrainFacade_H
