#include "Spark/CharacterInstance.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.CharacterInstance", CharacterInstance, Object)

CharacterInstance::CharacterInstance()
:	m_transform(Matrix33::identity())
{
}

void CharacterInstance::setTransform(const Matrix33& transform)
{
	m_transform = transform;
}

const Matrix33& CharacterInstance::getTransform() const
{
	return m_transform;
}

	}
}
