#include "Sound/Resound/RandomGrain.h"
#include "Sound/Editor/Resound/RandomGrainFacade.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.RandomGrainFacade", RandomGrainFacade, IGrainFacade)

bool RandomGrainFacade::canHaveChildren() const
{
	return true;
}

bool RandomGrainFacade::addChild(IGrain* parentGrain, IGrain* childGrain)
{
	checked_type_cast< RandomGrain*, false >(parentGrain)->addGrain(childGrain);
	return true;
}

bool RandomGrainFacade::removeChild(IGrain* parentGrain, IGrain* childGrain)
{
	checked_type_cast< RandomGrain*, false >(parentGrain)->removeGrain(childGrain);
	return true;
}

bool RandomGrainFacade::getChildren(IGrain* grain, RefArray< IGrain >& outChildren)
{
	outChildren = checked_type_cast< RandomGrain*, false >(grain)->getGrains();
	return true;
}

	}
}
