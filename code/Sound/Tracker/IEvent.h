#pragma once

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class AudioChannel;

class T_DLLCLASS IEvent : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool execute(AudioChannel* audioChannel, int32_t& bpm, int32_t& pattern, int32_t& row) const = 0;
};

	}
}
