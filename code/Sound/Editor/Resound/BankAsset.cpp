#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Sound/Sound.h"
#include "Sound/Resound/IGrainData.h"
#include "Sound/Editor/SoundCategory.h"
#include "Sound/Editor/Resound/BankAsset.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.sound.BankAsset", 2, BankAsset, ISerializable)

BankAsset::BankAsset()
:	m_presence(0.0f)
{
}

void BankAsset::addGrain(IGrainData* grain)
{
	m_grains.push_back(grain);
}

void BankAsset::removeGrain(IGrainData* grain)
{
	m_grains.remove(grain);
}

const RefArray< IGrainData >& BankAsset::getGrains() const
{
	return m_grains;
}

bool BankAsset::serialize(ISerializer& s)
{
	if (s.getVersion() >= 1)
		s >> Member< Guid >(L"category", m_category, AttributeType(type_of< SoundCategory >()));

	if (s.getVersion() >= 2)
		s >> Member< float >(L"presence", m_presence, AttributeRange(0.0f));

	return s >> MemberRefArray< IGrainData >(L"grains", m_grains);
}

	}
}
