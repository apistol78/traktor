#include "Spark/Character.h"
#include "Spark/CharacterBuilder.h"
#include "Spark/ICharacterFactory.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.CharacterBuilder", CharacterBuilder, ICharacterBuilder)

void CharacterBuilder::addFactory(ICharacterFactory* factory)
{
	TypeInfoSet typeSet = factory->getCharacterTypes();
	for (TypeInfoSet::const_iterator i = typeSet.begin(); i != typeSet.end(); ++i)
		m_factories[*i] = factory;
}

Ref< CharacterInstance > CharacterBuilder::create(const Context* context, const Character* character, const CharacterInstance* parent, const std::wstring& name) const
{
	SmallMap< const TypeInfo*, Ref< ICharacterFactory > >::const_iterator it = m_factories.find(&type_of(character));
	if (it != m_factories.end())
		return it->second->create(context, this, character, parent, name);
	else
		return 0;
}

	}
}
