#ifndef traktor_sound_BlendGrain_H
#define traktor_sound_BlendGrain_H

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
class T_DLLCLASS BlendGrain : public IGrain
{
	T_RTTI_CLASS;

public:
	BlendGrain(handle_t id, float response, IGrain* grain1, IGrain* grain2);

	virtual Ref< ISoundBufferCursor > createCursor() const;

	virtual void updateCursor(ISoundBufferCursor* cursor) const;

	virtual const IGrain* getCurrentGrain(const ISoundBufferCursor* cursor) const;

	virtual void getActiveGrains(const ISoundBufferCursor* cursor, RefArray< const IGrain >& outActiveGrains) const;

	virtual bool getBlock(ISoundBufferCursor* cursor, const ISoundMixer* mixer, SoundBlock& outBlock) const;

private:
	handle_t m_id;
	float m_response;
	Ref< IGrain > m_grains[2];
};

	}
}

#endif	// traktor_sound_BlendGrain_H
