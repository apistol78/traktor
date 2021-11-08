#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Sound/Types.h"
#include "Sound/Editor/SoundAsset.h"
#include "Sound/Editor/SoundCategory.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.sound.SoundAsset", 8, SoundAsset, editor::Asset)

void SoundAsset::serialize(ISerializer& s)
{
	editor::Asset::serialize(s);

	if (s.getVersion() >= 2)
		s >> Member< Guid >(L"category", m_category, AttributeType(type_of< SoundCategory >()));

	s >> Member< bool >(L"stream", m_stream);

	if (s.getVersion() >= 1)
		s >> Member< bool >(L"preload", m_preload);

	if (s.getVersion() >= 7)
		s >> Member< bool >(L"compressed", m_compressed);

	if (s.getVersion() < 8)
	{
		float presence;
		if (s.getVersion() >= 3)
			s >> Member< float >(L"presence", presence);

		float presenceRate;
		if (s.getVersion() >= 4)
			s >> Member< float >(L"presenceRate", presenceRate);
	}

	if (s.getVersion() >= 5)
	{
		if (s.getVersion() >= 6)
			s >> Member< float >(L"gain", m_gain, AttributeUnit(UnitType::Decibel));
		else
		{
			float volumeLin = 1.0f;
			s >> Member< float >(L"volume", volumeLin);
			m_gain = linearToDecibel(volumeLin);
		}
	}
}

	}
}
