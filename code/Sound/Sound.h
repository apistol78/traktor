#ifndef traktor_sound_Sound_H
#define traktor_sound_Sound_H

#include "Core/Object.h"
#include "Sound/Types.h"

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

class ISoundBuffer;

/*! \brief Sound container class.
 * \ingroup Sound
 */
class T_DLLCLASS Sound : public Object
{
	T_RTTI_CLASS;

public:
	Sound(ISoundBuffer* soundBuffer, float volume);

	ISoundBuffer* getSoundBuffer() const { return m_soundBuffer; }

	float getVolume() const { return m_volume; }

private:
	Ref< ISoundBuffer > m_soundBuffer;
	float m_volume;
};

	}
}

#endif	// traktor_sound_Sound_H
