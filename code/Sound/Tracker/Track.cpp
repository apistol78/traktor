#include "Sound/Tracker/Track.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.Track", Track, Object)

const Track::Key* Track::findKey(int32_t position) const
{
	for (const auto& key : m_keys)
	{
		if (key.at == position)
			return &key;
	}
	return nullptr;
}

	}
}
