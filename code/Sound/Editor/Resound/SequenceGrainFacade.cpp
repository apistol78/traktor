#include "Sound/Resound/SequenceGrain.h"
#include "Sound/Editor/Resound/SequenceGrainFacade.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SequenceGrainFacade", SequenceGrainFacade, IGrainFacade)

bool SequenceGrainFacade::canHaveChildren() const
{
	return true;
}

bool SequenceGrainFacade::addChild(IGrain* parentGrain, IGrain* childGrain)
{
	checked_type_cast< SequenceGrain*, false >(parentGrain)->addGrain(childGrain);
	return true;
}

bool SequenceGrainFacade::removeChild(IGrain* parentGrain, IGrain* childGrain)
{
	checked_type_cast< SequenceGrain*, false >(parentGrain)->removeGrain(childGrain);
	return true;
}

bool SequenceGrainFacade::getChildren(IGrain* grain, RefArray< IGrain >& outChildren)
{
	outChildren = checked_type_cast< SequenceGrain*, false >(grain)->getGrains();
	return true;
}

	}
}
