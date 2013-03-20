#ifndef traktor_sound_InLoopOutGrain_H
#define traktor_sound_InLoopOutGrain_H

#include "Core/Ref.h"
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
class T_DLLCLASS InLoopOutGrain : public IGrain
{
	T_RTTI_CLASS;

public:
	InLoopOutGrain(
		handle_t id,
		bool initial,
		IGrain* inGrain,
		IGrain* inLoopGrain,
		IGrain* outGrain,
		IGrain* outLoopGrain
	);

	virtual Ref< ISoundBufferCursor > createCursor() const;

	virtual void updateCursor(ISoundBufferCursor* cursor) const;

	virtual bool getBlock(ISoundBufferCursor* cursor, const ISoundMixer* mixer, SoundBlock& outBlock) const;

private:
	handle_t m_id;
	bool m_initial;
	Ref< IGrain > m_inGrain;
	Ref< IGrain > m_inLoopGrain;
	Ref< IGrain > m_outGrain;
	Ref< IGrain > m_outLoopGrain;
};

	}
}

#endif	// traktor_sound_InLoopOutGrain_H
