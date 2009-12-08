#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Resource/Member.h"
#include "Sound/ISoundResource.h"
#include "Sound/Editor/Resound/BankAsset.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.sound.BankAsset", 0, BankAsset, ITypedAsset)

const TypeInfo* BankAsset::getOutputType() const
{
	return &type_of< ISoundResource >();
}

bool BankAsset::serialize(ISerializer& s)
{
	return s >> MemberStlVector< resource::Proxy< Sound >, resource::Member< Sound, ISoundResource > >(L"sounds", m_sounds);
}

	}
}
