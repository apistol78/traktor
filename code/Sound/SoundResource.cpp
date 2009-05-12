#include "Sound/SoundResource.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundResource", SoundResource, Serializable)

bool SoundResource::serialize(Serializer& s)
{
	return true;
}

	}
}
