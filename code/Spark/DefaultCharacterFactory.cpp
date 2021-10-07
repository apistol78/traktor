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
	Context* context,
	Dictionary* dictionary,
	CharacterInstance* parent,
	const std::string& name,
	const Matrix33& transform
) const
{
	return character->createInstance(
		context,
		parent->getDictionary(),
		parent,
		name,
		transform
	);
}

void DefaultCharacterFactory::removeInstance(CharacterInstance* instance, int32_t depth) const
{
}

	}
}
