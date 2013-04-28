#ifndef traktor_sound_EnvelopeGrain_H
#define traktor_sound_EnvelopeGrain_H

#include "Core/RefArray.h"
#include "Core/Math/Envelope.h"
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
class T_DLLCLASS EnvelopeGrain : public IGrain
{
	T_RTTI_CLASS;

public:
	struct Grain
	{
		Ref< IGrain > grain;
		float in;
		float out;
		float easeIn;
		float easeOut;
	};

	EnvelopeGrain(handle_t id, const std::vector< Grain >& grains, const float levels[3], float mid);

	virtual Ref< ISoundBufferCursor > createCursor() const;

	virtual void updateCursor(ISoundBufferCursor* cursor) const;

	virtual const IGrain* getCurrentGrain(const ISoundBufferCursor* cursor) const;

	virtual bool getBlock(ISoundBufferCursor* cursor, const ISoundMixer* mixer, SoundBlock& outBlock) const;

private:
	handle_t m_id;
	std::vector< Grain > m_grains;
	Envelope< float, HermiteEvaluator< float > > m_envelope;
};

	}
}

#endif	// traktor_sound_EnvelopeGrain_H
