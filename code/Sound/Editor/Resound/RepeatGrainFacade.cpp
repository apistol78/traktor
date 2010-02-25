#include "Sound/Resound/RepeatGrain.h"
#include "Sound/Editor/Resound/RepeatGrainFacade.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.RepeatGrainFacade", RepeatGrainFacade, IGrainFacade)

bool RepeatGrainFacade::canHaveChildren() const
{
	return false;
}

bool RepeatGrainFacade::addChild(IGrain* parentGrain, IGrain* childGrain)
{
	return false;
}

bool RepeatGrainFacade::removeChild(IGrain* parentGrain, IGrain* childGrain)
{
	return false;
}

bool RepeatGrainFacade::getChildren(IGrain* grain, RefArray< IGrain >& outChildren)
{
	outChildren.push_back(checked_type_cast< RepeatGrain*, false >(grain)->getGrain());
	return true;
}

	}
}
