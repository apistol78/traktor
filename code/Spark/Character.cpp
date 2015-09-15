#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Spark/Character.h"
#include "Spark/IComponent.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Character", Character, ISerializable)

void Character::serialize(ISerializer& s)
{
	s >> Member< Matrix33 >(L"transform", m_transform);
	s >> MemberRefArray< IComponent >(L"components", m_components);
}

	}
}
