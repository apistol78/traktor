#ifndef traktor_sound_RandomGrain_H
#define traktor_sound_RandomGrain_H

#include "Core/RefArray.h"
#include "Core/Math/Random.h"
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

/*! \brief
 * \ingroup Sound
 */
class T_DLLCLASS RandomGrain : public IGrain
{
	T_RTTI_CLASS;

public:
	RandomGrain(
		const RefArray< IGrain >& grains,
		bool humanize
	);

	virtual Ref< ISoundBufferCursor > createCursor() const T_OVERRIDE T_FINAL;

	virtual void updateCursor(ISoundBufferCursor* cursor) const T_OVERRIDE T_FINAL;

	virtual const IGrain* getCurrentGrain(const ISoundBufferCursor* cursor) const T_OVERRIDE T_FINAL;

	virtual void getActiveGrains(const ISoundBufferCursor* cursor, RefArray< const IGrain >& outActiveGrains) const T_OVERRIDE T_FINAL;

	virtual bool getBlock(ISoundBufferCursor* cursor, const ISoundMixer* mixer, SoundBlock& outBlock) const T_OVERRIDE T_FINAL;

private:
	RefArray< IGrain > m_grains;
	bool m_humanize;
	mutable Random m_random;
	mutable int32_t m_last;
};

	}
}

#endif	// traktor_sound_RandomGrain_H
