#include "Spark/Character.h"
#include "Spark/ICharacterFactory.h"
#include "Spark/Editor/CharacterAdapter.h"
#include "Spark/Editor/CharacterAdapterBuilder.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.CharacterAdapterBuilder", CharacterAdapterBuilder, ICharacterBuilder)

CharacterAdapterBuilder::CharacterAdapterBuilder(Ref< CharacterAdapter >& outRoot, RefArray< CharacterAdapter >& outAdapters)
:	m_outRoot(outRoot)
,	m_outAdapters(outAdapters)
{
}

void CharacterAdapterBuilder::addFactory(ICharacterFactory* factory)
{
	TypeInfoSet typeSet = factory->getCharacterTypes();
	for (TypeInfoSet::const_iterator i = typeSet.begin(); i != typeSet.end(); ++i)
		m_factories[*i] = factory;
}

Ref< CharacterInstance > CharacterAdapterBuilder::create(const Character* character, const CharacterInstance* parent, const std::wstring& name) const
{
	SmallMap< const TypeInfo*, Ref< ICharacterFactory > >::const_iterator it = m_factories.find(&type_of(character));
	if (it == m_factories.end())
		return 0;

	Ref< CharacterAdapter > adapter = new CharacterAdapter();
	m_outAdapters.push_back(adapter);

	if (!m_adapterStack.empty())
	{
		adapter->m_parent = m_adapterStack.back();
		m_adapterStack.back()->m_children.push_back(adapter);
	}

	m_adapterStack.push_back(adapter);
	Ref< CharacterInstance > instance = it->second->create(this, character, parent, name);
	m_adapterStack.pop_back();

	adapter->m_name = name;
	adapter->m_character = const_cast< Character* >(character);
	adapter->m_characterInstance = instance;

	if (!adapter->m_parent)
		m_outRoot = adapter;

	return instance;
}

	}
}
