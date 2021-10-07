#include "Spark/Character.h"
#include "Spark/Types.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Character", Character, ISerializable)

Character::Character()
:	m_tag(allocateCacheTag())
{
}

void Character::serialize(ISerializer& s)
{
}

	}
}
