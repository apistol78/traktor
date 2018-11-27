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

class SoundChannel;

class T_DLLCLASS IEvent : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool execute(SoundChannel* soundChannel, int32_t& bpm, int32_t& pattern, int32_t& row) const = 0;
};

	}
}
