#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spark/Character.h"
#include "Spark/Types.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Character", Character, ISerializable)

Character::Character()
:	m_id(0)
,	m_tag(allocateCacheTag())
{
}

Character::Character(uint16_t id)
:	m_id(id)
,	m_tag(allocateCacheTag())
{
}

void Character::serialize(ISerializer& s)
{
	s >> Member< uint16_t >(L"id", m_id);
}

	}
}
