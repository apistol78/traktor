/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_Sound_H
#define traktor_sound_Sound_H

#include "Core/Object.h"
#include "Core/Ref.h"
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
		handle_t category,
		float gain,
		float presence,
		float presenceRate,
		float range
	);

	ISoundBuffer* getBuffer() const { return m_buffer; }

	uint32_t getCategory() const { return m_category; }

	float getGain() const { return m_gain; }

	float getPresence() const { return m_presence; }

	float getPresenceRate() const { return m_presenceRate; }

	float getRange() const { return m_range; }

private:
	Ref< ISoundBuffer > m_buffer;
	handle_t m_category;
	float m_gain;
	float m_presence;
	float m_presenceRate;
	float m_range;
};

	}
}

#endif	// traktor_sound_Sound_H
