/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Spark/CharacterBuilder.h"
#include "Spark/CharacterData.h"
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

Ref< Character > CharacterBuilder::create(const Context* context, const CharacterData* characterData, const Character* parent, const std::wstring& name) const
{
	SmallMap< const TypeInfo*, Ref< ICharacterFactory > >::const_iterator it = m_factories.find(&type_of(characterData));
	if (it != m_factories.end())
		return it->second->create(context, this, characterData, parent, name);
	else
		return 0;
}

	}
}
