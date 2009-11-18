#include "Sound/Editor/SoundAsset.h"
#include "Sound/SoundResource.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.sound.SoundAsset", SoundAsset, editor::Asset)

SoundAsset::SoundAsset()
:	m_stream(false)
{
}

const TypeInfo* SoundAsset::getOutputType() const
{
	return &type_of< SoundResource >();
}

bool SoundAsset::serialize(ISerializer& s)
{
	if (!editor::Asset::serialize(s))
		return false;

	s >> Member< bool >(L"stream", m_stream);

	return true;
}

	}
}
