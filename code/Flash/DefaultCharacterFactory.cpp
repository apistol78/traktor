#include "Flash/DefaultCharacterFactory.h"
#include "Flash/FlashCharacter.h"
#include "Flash/FlashCharacterInstance.h"

namespace traktor
{
	namespace flash
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.DefaultCharacterFactory", DefaultCharacterFactory, ICharacterFactory)
	
Ref< FlashCharacterInstance > DefaultCharacterFactory::createInstance(
	const FlashCharacter* character,
	int32_t depth,
	ActionContext* context,
	FlashDictionary* dictionary,
	FlashCharacterInstance* parent,
	const std::string& name,
	const Matrix33& transform,
	const ActionObject* initObject,
	const SmallMap< uint32_t, Ref< const IActionVMImage > >* events
) const
{
	return character->createInstance(
		context,
		parent->getDictionary(),
		parent,
		name,
		transform,
		initObject,
		events
	);
}

void DefaultCharacterFactory::removeInstance(FlashCharacterInstance* instance, int32_t depth) const
{
}

	}
}
