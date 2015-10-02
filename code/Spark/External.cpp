#include "Core/Serialization/ISerializer.h"
#include "Resource/Member.h"
#include "Spark/External.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.External", 0, External, Character)

External::External()
{
}

External::External(const resource::Id< Character >& reference)
:	m_reference(reference)
{
}

void External::serialize(ISerializer& s)
{
	Character::serialize(s);
	s >> resource::Member< Character >(L"reference", m_reference);
}

	}
}
