#include "Core/Serialization/ISerializer.h"
#include "Resource/Member.h"
#include "Spark/ExternalData.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.spark.ExternalData", 0, ExternalData, CharacterData)

ExternalData::ExternalData()
{
}

ExternalData::ExternalData(const resource::Id< CharacterData >& reference)
:	m_reference(reference)
{
}

void ExternalData::serialize(ISerializer& s)
{
	CharacterData::serialize(s);
	s >> resource::Member< CharacterData >(L"reference", m_reference);
}

	}
}
