#include "Spark/DefaultCharacterFactory.h"
#include "Spark/Character.h"
#include "Spark/CharacterInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.DefaultCharacterFactory", DefaultCharacterFactory, ICharacterFactory)

Ref< CharacterInstance > DefaultCharacterFactory::createInstance(
	const Character* character,
	int32_t depth,
	ActionContext* context,
	Dictionary* dictionary,
	CharacterInstance* parent,
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

void DefaultCharacterFactory::removeInstance(CharacterInstance* instance, int32_t depth) const
{
}

	}
}
