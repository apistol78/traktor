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
	Sound(
		ISoundBuffer* buffer,
		float volume,
		float presence,
		float presenceRate,
		float range
	);

	ISoundBuffer* getBuffer() const { return m_buffer; }

	float getVolume() const { return m_volume; }

	float getPresence() const { return m_presence; }

	float getPresenceRate() const { return m_presenceRate; }

	float getRange() const { return m_range; }

private:
	Ref< ISoundBuffer > m_buffer;
	float m_volume;
	float m_presence;
	float m_presenceRate;
	float m_range;
};

	}
}

#endif	// traktor_sound_Sound_H
