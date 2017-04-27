/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAabb.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Resource/Member.h"
#include "Spark/IComponentData.h"
#include "Spark/Shape.h"
#include "Spark/SpriteData.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

class MemberNamedCharacter : public MemberComplex
{
public:
	MemberNamedCharacter(const wchar_t* const name, SpriteData::NamedCharacter& ref)
	:	MemberComplex(name, true)
	,	m_ref(ref)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		s >> Member< std::wstring >(L"name", m_ref.name);
		s >> MemberRef< CharacterData >(L"character", m_ref.character);
	}

private:
	SpriteData::NamedCharacter& m_ref;
};

class FindNamedCharacter
{
public:
	FindNamedCharacter(const std::wstring& name)
	:	m_name(name)
	{
	}

	bool operator () (const SpriteData::NamedCharacter& nc) const
	{
		return nc.name == m_name;
	}

private:
	std::wstring m_name;
};

class FindCharacter
{
public:
	FindCharacter(const CharacterData* character)
	:	m_character(character)
	{
	}

	bool operator () (const SpriteData::NamedCharacter& nc) const
	{
		return nc.character == m_character;
	}

private:
	const CharacterData* m_character;
};

		}

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.SpriteData", 0, SpriteData, CharacterData)

const CharacterData* SpriteData::getCharacter(const std::wstring& id) const
{
	AlignedVector< NamedCharacter >::const_iterator i = std::find_if(m_dictionary.begin(), m_dictionary.end(), FindNamedCharacter(id));
	return i != m_dictionary.end() ? i->character : 0;
}

void SpriteData::place(const std::wstring& name, CharacterData* character)
{
	NamedCharacter nc;
	nc.name = name;
	nc.character = character;
	m_frame.push_back(nc);
}

void SpriteData::remove(CharacterData* character)
{
	AlignedVector< NamedCharacter >::iterator i = std::find_if(m_frame.begin(), m_frame.end(), FindCharacter(character));
	if (i != m_frame.end())
		m_frame.erase(i);
}

void SpriteData::serialize(ISerializer& s)
{
	CharacterData::serialize(s);

	s >> MemberRefArray< IComponentData >(L"components", m_components);
	s >> MemberAabb2(L"bounds", m_bounds);
	s >> resource::Member< Shape >(L"shape", m_shape);
	s >> MemberAlignedVector< NamedCharacter, MemberNamedCharacter >(L"dictionary", m_dictionary);
	s >> MemberAlignedVector< NamedCharacter, MemberNamedCharacter >(L"frame", m_frame);
}

	}
}
