#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spark/Character.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.Character", Character, ISerializable)

Character::Character()
:	m_transform(Matrix33::identity())
{
}

void Character::setTransform(const Matrix33& transform)
{
	m_transform = transform;
}

const Matrix33& Character::getTransform() const
{
	return m_transform;
}

void Character::serialize(ISerializer& s)
{
	s >> Member< Matrix33 >(L"transform", m_transform);
}

	}
}
