#include "Sound/SoundResource.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundResource", SoundResource, ISerializable)

bool SoundResource::serialize(ISerializer& s)
{
	return true;
}

	}
}
