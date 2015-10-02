#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Resource/Member.h"
#include "Spark/IComponent.h"
#include "Spark/Shape.h"
#include "Spark/Sprite.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

class MemberNamedCharacter : public MemberComplex
{
public:
	MemberNamedCharacter(const wchar_t* const name, Sprite::NamedCharacter& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		s >> Member< std::wstring >(L"name", m_ref.name);
		s >> MemberRef< Character >(L"character", m_ref.character);
	}

private:
	Sprite::NamedCharacter& m_ref;
};

class FindNamedCharacter
{
public:
	FindNamedCharacter(const std::wstring& name)
	:	m_name(name)
	{
	}

	bool operator () (const Sprite::NamedCharacter& nc) const
	{
		return nc.name == m_name;
	}

private:
	std::wstring m_name;
};

		}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.Sprite", 0, Sprite, Character)

const Character* Sprite::getCharacter(const std::wstring& id) const
{
	AlignedVector< NamedCharacter >::const_iterator i = std::find_if(m_dictionary.begin(), m_dictionary.end(), FindNamedCharacter(id));
	return i != m_dictionary.end() ? i->character : 0;
}

void Sprite::place(const std::wstring& name, Character* character)
{
	NamedCharacter nc;
	nc.name = name;
	nc.character = character;
	m_frame.push_back(nc);
}

void Sprite::serialize(ISerializer& s)
{
	Character::serialize(s);

	s >> MemberRefArray< IComponent >(L"components", m_components);
	s >> resource::Member< Shape >(L"shape", m_shape);
	s >> MemberAlignedVector< NamedCharacter, MemberNamedCharacter >(L"dictionary", m_dictionary);
	s >> MemberAlignedVector< NamedCharacter, MemberNamedCharacter >(L"frame", m_frame);
}

	}
}
