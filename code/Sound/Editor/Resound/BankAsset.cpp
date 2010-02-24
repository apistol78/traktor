#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Sound/ISoundResource.h"
#include "Sound/Resound/IGrain.h"
#include "Sound/Editor/Resound/BankAsset.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.sound.BankAsset", 0, BankAsset, ITypedAsset)

void BankAsset::addGrain(IGrain* grain)
{
	m_grains.push_back(grain);
}

const RefArray< IGrain >& BankAsset::getGrains() const
{
	return m_grains;
}

const TypeInfo* BankAsset::getOutputType() const
{
	return &type_of< ISoundResource >();
}

bool BankAsset::serialize(ISerializer& s)
{
	return s >> MemberRefArray< IGrain >(L"grains", m_grains);
}

	}
}
