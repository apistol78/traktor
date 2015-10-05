#ifndef traktor_sound_PlayGrain_H
#define traktor_sound_PlayGrain_H

#include "Core/RefArray.h"
#include "Core/Math/Range.h"
#include "Resource/Proxy.h"
#include "Sound/Resound/IGrain.h"

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

class IFilter;
class Sound;

/*! \brief
 * \ingroup Sound
 */
class T_DLLCLASS PlayGrain : public IGrain
{
	T_RTTI_CLASS;

public:
	PlayGrain(
		const resource::Proxy< Sound >& sound,
		const RefArray< IFilter >& filters,
		const Range< float >& gain,
		const Range< float >& pitch,
		bool repeat
	);

	virtual Ref< ISoundBufferCursor > createCursor() const T_OVERRIDE T_FINAL;

	virtual void updateCursor(ISoundBufferCursor* cursor) const T_OVERRIDE T_FINAL;

	virtual const IGrain* getCurrentGrain(const ISoundBufferCursor* cursor) const T_OVERRIDE T_FINAL;

	virtual void getActiveGrains(const ISoundBufferCursor* cursor, RefArray< const IGrain >& outActiveGrains) const T_OVERRIDE T_FINAL;

	virtual bool getBlock(ISoundBufferCursor* cursor, const ISoundMixer* mixer, SoundBlock& outBlock) const T_OVERRIDE T_FINAL;

private:
	resource::Proxy< Sound > m_sound;
	RefArray< IFilter > m_filters;
	Range< float > m_gain;
	Range< float > m_pitch;
	bool m_repeat;
	mutable Random m_random;
};

	}
}

#endif	// traktor_sound_PlayGrain_H
