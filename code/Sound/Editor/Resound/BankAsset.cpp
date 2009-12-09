#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Sound/ISoundResource.h"
#include "Sound/Resound/BankSound.h"
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

void BankAsset::addSound(BankSound* sound)
{
	m_sounds.push_back(sound);
}

const RefArray< BankSound >& BankAsset::getSounds() const
{
	return m_sounds;
}

const TypeInfo* BankAsset::getOutputType() const
{
	return &type_of< ISoundResource >();
}

bool BankAsset::serialize(ISerializer& s)
{
	s >> MemberRefArray< IGrain >(L"grains", m_grains);
	s >> MemberRefArray< BankSound >(L"sounds", m_sounds);
	return true;
}

	}
}
